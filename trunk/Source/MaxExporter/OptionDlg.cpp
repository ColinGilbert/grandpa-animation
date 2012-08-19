///////////////////////////////////////////////////////////////////////////////
//导出设置对话框
//
//描述：
//
//历史：
//		08-01-31	创建
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "OptionDlg.h"
#include "Exporter.h"
#include "Resource.h"
#include <map>

using std::vector;
using std::map;
using std::make_pair;

enum
{
	PAGE_SKELETON	= 0,
	PAGE_ANIMATION	= 1,
	PAGE_MESH		= 2,
};

#define	UM_CHECKSTATECHANGE		( WM_USER + 100 )

extern HINSTANCE g_hInstance;

BOOL CALLBACK SkeletonDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK AnimationDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK MeshDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK BspDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

void SetPageState( DWORD dwExpType, IRollupWindow* pRollup );
void ShowPage( IRollupWindow* pRollup, int iIndex, BOOL bShow );

void BuildTreeViewForNodes( HWND hTreeView, vector<INode*>* pVecNode );

void SetTreeAncestorCheck( HWND hTreeView, HTREEITEM hItem );
void SetTreeCheckState( HWND hTreeView, HTREEITEM hItem, BOOL bCheck );

void GetSelectedNodeInTreeView( HWND hTreeView, vector<INode*>* pVecNode );
void GetTreeNode( HWND hTreeView, HTREEITEM hItem, vector<INode*>* pVecNode );

void saveSettings( const ExportOptions& options )
{
	wchar_t out[32];
	swprintf( out, sizeof(out), L"%d", options.exportType );
	::WritePrivateProfileStringW( L"grandpa exporter", L"type", out, L"GrandpaMax.ini" );
	swprintf( out, sizeof(out), L"%f", options.positionTolerance );
	::WritePrivateProfileStringW( L"grandpa exporter", L"position tolerance", out, L"GrandpaMax.ini" );
	swprintf( out, sizeof(out), L"%f", options.rotationTolerance );
	::WritePrivateProfileStringW( L"grandpa exporter", L"rotation tolerance", out, L"GrandpaMax.ini" );
	swprintf( out, sizeof(out), L"%f", options.scaleTolerance );
	::WritePrivateProfileStringW( L"grandpa exporter", L"scale tolerance", out, L"GrandpaMax.ini" );
	swprintf( out, sizeof(out), L"%f", options.lodLevelScale );
	::WritePrivateProfileStringW( L"grandpa exporter", L"lod level scale", out, L"GrandpaMax.ini" );
	swprintf( out, sizeof(out), L"%f", options.lodMaxError );
	::WritePrivateProfileStringW( L"grandpa exporter", L"lod max error", out, L"GrandpaMax.ini" );
}

void initOptionDlg( HWND hWnd, ExportOptions* options )
{
	assert( options != NULL );
	CheckDlgButton( hWnd, IDC_CHECK_SKELETON,
		( ( options->exportType & EXP_SKELETON ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_ANIMATION,
		( ( options->exportType & EXP_ANIMATION ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_MESH,
		( ( options->exportType & EXP_MESH ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );

	IRollupWindow* pRollup = GetIRollup( GetDlgItem( hWnd, IDC_ROLLUP_WND ) );
	if ( pRollup )
	{
		if ( pRollup->GetNumPanels() > 0 )
		{
			pRollup->DeleteRollup( 0, pRollup->GetNumPanels() );
		}
		int index;
		index = pRollup->AppendRollup( g_hInstance,
			MAKEINTRESOURCE( IDD_PANEL_SKELETON ),
			(DLGPROC)SkeletonDlgProc,
			"Skeleton Settings", (LPARAM)options );
		pRollup->SetPanelOpen( index, FALSE );

		index = pRollup->AppendRollup( g_hInstance,
			MAKEINTRESOURCE( IDD_PANEL_ANIMATION ),
			(DLGPROC)AnimationDlgProc,
			"Animation Settings", (LPARAM)options );
		pRollup->SetPanelOpen( index, FALSE );

		index = pRollup->AppendRollup( g_hInstance,
			MAKEINTRESOURCE( IDD_PANEL_MESH ),
			(DLGPROC)MeshDlgProc,
			"Mesh Settings", (LPARAM)options);
		pRollup->SetPanelOpen( index, TRUE );

		pRollup->Show();

		SetPageState( options->exportType, pRollup );
	}
}

void initAnimationDlg( HWND hWnd, ExportOptions* options )
{
	CheckDlgButton( hWnd, IDC_RADIO_STEP,
		( ( options->exportType & EXP_ANIM_SAMPLE_MASK ) == EXP_ANIM_SAMPLE_STEP ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_RADIO_LINEAR,
		( ( options->exportType & EXP_ANIM_SAMPLE_MASK ) == EXP_ANIM_SAMPLE_LINEAR ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_RADIO_SPLINE,
		( ( options->exportType & EXP_ANIM_SAMPLE_MASK ) == EXP_ANIM_SAMPLE_SPLINE ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_QUATERNION,
		( ( options->exportType & EXP_ANIM_COMPRESS_QUATERNION ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );

	char threshold[32];
	sprintf_s( threshold, sizeof(threshold), "%f", options->positionTolerance );
	SetDlgItemTextA( hWnd, IDC_EDIT_THRESHOLD_POSITION, threshold );
	sprintf_s( threshold, sizeof(threshold), "%f", options->rotationTolerance );
	SetDlgItemTextA( hWnd, IDC_EDIT_THRESHOLD_ROTATION, threshold );
	sprintf_s( threshold, sizeof(threshold), "%f", options->scaleTolerance );
	SetDlgItemTextA( hWnd, IDC_EDIT_THRESHOLD_SCALE, threshold );
}

void initMeshDlg( HWND hWnd, ExportOptions* options )
{
	CheckDlgButton( hWnd, IDC_CHECK_POSITION, BST_CHECKED );

	if ( ( options->exportType & EXP_MESH_NORMAL ) == 0
		|| ( options->exportType & EXP_MESH_TEXCOORD ) == 0 )
	{
		options->exportType &= ~EXP_MESH_TANGENT;
		EnableWindow( GetDlgItem( hWnd, IDC_CHECK_TANGENT ), FALSE );
	}
	CheckDlgButton( hWnd, IDC_CHECK_NORMAL,
		( ( options->exportType & EXP_MESH_NORMAL ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_TANGENT,
		( ( options->exportType & EXP_MESH_TANGENT ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_TEXCOORD,
		( ( options->exportType & EXP_MESH_TEXCOORD ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_COLOR,
		( ( options->exportType & EXP_MESH_VERTEX_COLOR ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_LOD,
		( ( options->exportType & EXP_MESH_LOD ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_POS,
		( ( options->exportType & EXP_MESH_COMPRESS_POS ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_NORMAL,
		( ( options->exportType & EXP_MESH_COMPRESS_NORMAL ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_TEXCOORD,
		( ( options->exportType & EXP_MESH_COMPRESS_TEXCOORD ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_WEIGHT,
		( ( options->exportType & EXP_MESH_COMPRESS_WEIGHT ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_MERGE,
		( ( options->exportType & EXP_MESH_MERGEBUFFER ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hWnd, IDC_CHECK_PROPERTY,
		( ( options->exportType & EXP_MESH_PROPERTY ) != 0 ) ? BST_CHECKED : BST_UNCHECKED );

	char str[32];
	sprintf_s( str, sizeof(str), "%f", options->lodLevelScale );
	SetDlgItemTextA( hWnd, IDC_EDIT_LEVELSCALE, str );
	sprintf_s( str, sizeof(str), "%f", options->lodMaxError );
	SetDlgItemTextA( hWnd, IDC_EDIT_MAXERROR, str );

	BOOL enable = ( options->exportType & EXP_MESH_LOD ) != 0;
	EnableWindow( GetDlgItem( hWnd, IDC_STATIC_LODSCALE ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_STATIC_MAXERROR ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_EDIT_LEVELSCALE ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_EDIT_MAXERROR ), enable );
}

void EnableTreeViewCheckbox( HWND hTree )
{
	LONG dwStyle = GetWindowLong( hTree, GWL_STYLE );
	dwStyle &= ~(TVS_CHECKBOXES);
	SetWindowLong( hTree, GWL_STYLE, dwStyle );

	dwStyle |= TVS_CHECKBOXES;
	SetWindowLong( hTree, GWL_STYLE, dwStyle );
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK OptionDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static ExportOptions* options = NULL;

	switch ( message )
	{
	case WM_INITDIALOG:
		{
			options = (ExportOptions*)lParam;
			initOptionDlg( hWnd, options );
		}
		break;

	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_BUTTON_RESET:
			if ( ::MessageBoxW( hWnd, L"Are you sure to restore default settings?", L"Grandpa", MB_ICONWARNING | MB_YESNO )
				== IDYES )
			{
				options->exportType = DEFAULT_SETTINGS;
				options->positionTolerance = 0.001f;
				options->rotationTolerance = 0.001f;
				options->scaleTolerance = 0.001f;
				options->lodLevelScale = 0.4f;
				options->lodMaxError = 0.1f;

				initOptionDlg( hWnd, options );

				IRollupWindow* pRollup = GetIRollup( GetDlgItem( hWnd, IDC_ROLLUP_WND ) );
				assert( pRollup != NULL );
				initAnimationDlg( pRollup->GetPanelDlg( PAGE_ANIMATION ), options );
				initMeshDlg( pRollup->GetPanelDlg( PAGE_MESH ), options );
			}
			break;

		case IDC_CHECK_SKELETON:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_SKELETON;
			}
			else
			{
				options->exportType &= ~EXP_SKELETON;
			}
			{
				IRollupWindow* pRollup = GetIRollup( GetDlgItem( hWnd, IDC_ROLLUP_WND ) );
				if ( pRollup )
				{
					SetPageState( options->exportType, pRollup );
				}
			}
			break;

		case IDC_CHECK_ANIMATION:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_ANIMATION;
			}
			else
			{
				options->exportType &= ~EXP_ANIMATION;
			}
			{
				IRollupWindow* pRollup = GetIRollup( GetDlgItem( hWnd, IDC_ROLLUP_WND ) );
				if ( pRollup )
				{
					SetPageState( options->exportType, pRollup );
				}
			}
			break;

		case IDC_CHECK_MESH:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH;
			}
			else
			{
				options->exportType &= ~EXP_MESH;
			}
			{
				IRollupWindow* pRollup = GetIRollup( GetDlgItem( hWnd, IDC_ROLLUP_WND ) );
				if ( pRollup )
				{
					SetPageState( options->exportType, pRollup );
				}
			}
			break;
		case IDC_OK:
			{
				IRollupWindow* pRollup = GetIRollup( GetDlgItem( hWnd, IDC_ROLLUP_WND ) );

				//骨骼列表
				HWND hPanelSkeleton = pRollup->GetPanelDlg( PAGE_SKELETON );
				GetSelectedNodeInTreeView( GetDlgItem( hPanelSkeleton, IDC_TREE_BONE ),
					options->skeletonBoneNodes );
				if ( ( options->exportType & EXP_SKELETON ) != 0
					&& options->skeletonBoneNodes->empty() )
				{
					MessageBox( hWnd, "No bone selected.", "Skeleton",
								MB_OK | MB_ICONWARNING );
					break;
				}

				//动作骨骼列表
				HWND hPanelAnimation = pRollup->GetPanelDlg( PAGE_ANIMATION );
				GetSelectedNodeInTreeView( GetDlgItem( hPanelAnimation, IDC_TREE_ANIMATION ),
											options->animationBoneNodes );
				char threshold[32];
				GetDlgItemTextA( hPanelAnimation, IDC_EDIT_THRESHOLD_POSITION, threshold, sizeof(threshold) );
				sscanf_s( threshold, "%f", &options->positionTolerance );
				GetDlgItemTextA( hPanelAnimation, IDC_EDIT_THRESHOLD_ROTATION, threshold, sizeof(threshold) );
				sscanf_s( threshold, "%f", &options->rotationTolerance );
				GetDlgItemTextA( hPanelAnimation, IDC_EDIT_THRESHOLD_SCALE, threshold, sizeof(threshold) );
				sscanf_s( threshold, "%f", &options->scaleTolerance );
				if ( ( options->exportType & EXP_ANIMATION ) != 0
					&& options->animationBoneNodes->empty() )
				{
					MessageBox( hWnd, "No bone selected.", "Animation",
						MB_OK | MB_ICONWARNING );
					break;
				}

				//网格列表
				//如果要导出的是bsp，则使用bsp设置里的mesh列表，否则使用mesh设置里的mesh列表
				HWND hTreeView = GetDlgItem( pRollup->GetPanelDlg( PAGE_MESH ), IDC_TREE_MESH );
				GetSelectedNodeInTreeView( hTreeView, options->meshNodes );
				if ( ( options->exportType & EXP_MESH ) != 0 && options->meshNodes->empty() )
				{ 
					MessageBox( hWnd, "No mesh selected.", "Mesh",
						MB_OK | MB_ICONWARNING );
					break;
				}
				HWND hPanelMesh = pRollup->GetPanelDlg( PAGE_MESH );
				char str[32];
				GetDlgItemTextA( hPanelMesh, IDC_EDIT_LEVELSCALE, str, sizeof(str) );
				sscanf_s( str, "%f", &options->lodLevelScale );
				GetDlgItemTextA( hPanelMesh, IDC_EDIT_MAXERROR, str, sizeof(str) );
				sscanf_s( str, "%f", &options->lodMaxError );
			}
			saveSettings( *options );
			EndDialog( hWnd, 1 );
			break;

		case IDC_CANCEL:
			EndDialog( hWnd, 0 );
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog( hWnd, 0 );
		break;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK SkeletonDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static ExportOptions* options = NULL;

	switch ( message )
	{
	case WM_INITDIALOG:
		options = (ExportOptions*)lParam;
		EnableTreeViewCheckbox( GetDlgItem( hWnd, IDC_TREE_BONE ) );
		BuildTreeViewForNodes( GetDlgItem( hWnd, IDC_TREE_BONE ), options->skeletonBoneNodes );
		break;

	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_CHECK_PROPERTY:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_SKEL_PROPERTY;
			}
			else
			{
				options->exportType &= ~EXP_SKEL_PROPERTY;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		{   
			LPNMHDR lpnmh = (LPNMHDR)lParam;   
			TVHITTESTINFO ht = {0};   

			if ( lpnmh->code == NM_CLICK && ( lpnmh->idFrom == IDC_TREE_BONE ) )   
			{   
				DWORD dwPos = GetMessagePos();   
				ht.pt.x = GET_X_LPARAM( dwPos );
				ht.pt.y = GET_Y_LPARAM( dwPos );
				MapWindowPoints( HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1 );   

				TreeView_HitTest( lpnmh->hwndFrom, &ht );

				if ( TVHT_ONITEMSTATEICON & ht.flags )
				{
					PostMessage( hWnd, UM_CHECKSTATECHANGE, 0, (LPARAM)ht.hItem );
				}
			}
		}
		break;   

	case UM_CHECKSTATECHANGE:
		{
			HWND hTreeView = GetDlgItem( hWnd, IDC_TREE_BONE );
			HTREEITEM hItem = (HTREEITEM)lParam;
			BOOL bCheck = TreeView_GetCheckState( hTreeView, hItem );
			if ( bCheck )
			{
				SetTreeAncestorCheck( hTreeView, hItem );
			}
			SetTreeCheckState( hTreeView, hItem, bCheck );
		}
		break;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK AnimationDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static ExportOptions* options = NULL;

	switch ( message )
	{
	case WM_INITDIALOG:
		options = (ExportOptions*)lParam;

		EnableTreeViewCheckbox( GetDlgItem( hWnd, IDC_TREE_ANIMATION ) );
		BuildTreeViewForNodes( GetDlgItem( hWnd, IDC_TREE_ANIMATION ), options->animationBoneNodes );

		initAnimationDlg( hWnd, options );
		break;

	case WM_NOTIFY:
		{   
			LPNMHDR lpnmh = (LPNMHDR)lParam;   
			TVHITTESTINFO ht = {0};   

			if ( lpnmh->code == NM_CLICK && ( lpnmh->idFrom == IDC_TREE_ANIMATION ) )   
			{
				DWORD dwPos = GetMessagePos();
				ht.pt.x = GET_X_LPARAM( dwPos );
				ht.pt.y = GET_Y_LPARAM( dwPos );
				MapWindowPoints( HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1 );

				TreeView_HitTest( lpnmh->hwndFrom, &ht );

				if ( TVHT_ONITEMSTATEICON & ht.flags )   
				{
					PostMessage( hWnd, UM_CHECKSTATECHANGE, 0, (LPARAM)ht.hItem );
				}
			}
		}
		break;   

	case UM_CHECKSTATECHANGE:
		{
			HWND hTreeView = GetDlgItem( hWnd, IDC_TREE_ANIMATION );
			HTREEITEM hItem = (HTREEITEM)lParam;
			BOOL bCheck = TreeView_GetCheckState( hTreeView, hItem );
			SetTreeCheckState( hTreeView, hItem, bCheck );
		}
		break;

	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_RADIO_STEP:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType &= ~EXP_ANIM_SAMPLE_MASK;
				options->exportType |= EXP_ANIM_SAMPLE_STEP;
			}
			break;
		case IDC_RADIO_LINEAR:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType &= ~EXP_ANIM_SAMPLE_MASK;
				options->exportType |= EXP_ANIM_SAMPLE_LINEAR;
			}
			break;
		case IDC_RADIO_SPLINE:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType &= ~EXP_ANIM_SAMPLE_MASK;
				options->exportType |= EXP_ANIM_SAMPLE_SPLINE;
			}
			break;

		case IDC_CHECK_COMPRESS_QUATERNION:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				char temp[32];
				float rotationThreshold;
				GetDlgItemTextA( hWnd, IDC_EDIT_THRESHOLD_ROTATION, temp, sizeof(temp) );
				sscanf_s( temp, "%f", &rotationThreshold );
				if ( rotationThreshold < 0.0015f )
				{
					//压缩四元数最大可能引入约0.0015的误差
					SetDlgItemTextA( hWnd, IDC_EDIT_THRESHOLD_ROTATION, "0.0015" );
				}
				options->exportType |= EXP_ANIM_COMPRESS_QUATERNION;
			}
			else
			{
				options->exportType &= ~EXP_ANIM_COMPRESS_QUATERNION;
			}
			break;

		case IDC_EDIT_THRESHOLD_ROTATION:
			{
				char temp[32];
				float rotationThreshold;
				GetDlgItemTextA( hWnd, IDC_EDIT_THRESHOLD_ROTATION, temp, sizeof(temp) );
				sscanf_s( temp, "%f", &rotationThreshold );
				//压缩四元数最大可能引入约0.0015的误差
				if ( rotationThreshold < 0.0015f )
				{
					CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_QUATERNION, FALSE );
					options->exportType &= ~EXP_ANIM_COMPRESS_QUATERNION;
				}
				else
				{
					CheckDlgButton( hWnd, IDC_CHECK_COMPRESS_QUATERNION, TRUE );
					options->exportType |= EXP_ANIM_COMPRESS_QUATERNION;
				}
			}
		}
		break;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK MeshDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static ExportOptions* options = NULL;

	switch ( message )
	{
	case WM_INITDIALOG:
		options = (ExportOptions*)lParam;

		EnableTreeViewCheckbox( GetDlgItem( hWnd, IDC_TREE_MESH ) );
		BuildTreeViewForNodes( GetDlgItem( hWnd, IDC_TREE_MESH ), options->meshNodes );

		initMeshDlg( hWnd, options );
		break;

	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_CHECK_POSITION:
			CheckDlgButton( hWnd, LOWORD( wParam ), BST_CHECKED );
			break;
		case IDC_CHECK_NORMAL:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_NORMAL;
			}
			else
			{
				options->exportType &= ~EXP_MESH_NORMAL;
				if ( ( options->exportType & EXP_MESH_TANGENT ) != 0 )
				{
					options->exportType &= ~EXP_MESH_TANGENT;
					CheckDlgButton( hWnd, IDC_CHECK_TANGENT, BST_UNCHECKED );
				}
			}
			break;
		case IDC_CHECK_TANGENT:
			//必须有normal和texcoord，否则tangent就没有意义
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_TANGENT;
				options->exportType |= EXP_MESH_NORMAL;
				options->exportType |= EXP_MESH_TEXCOORD;
				CheckDlgButton( hWnd, IDC_CHECK_NORMAL, BST_CHECKED );
				CheckDlgButton( hWnd, IDC_CHECK_TEXCOORD, BST_CHECKED );
			}
			else
			{
				options->exportType &= ~EXP_MESH_TANGENT;
			}
			break;
		case IDC_CHECK_TEXCOORD:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_TEXCOORD;
			}
			else
			{
				options->exportType &= ~EXP_MESH_TEXCOORD;
				if ( ( options->exportType & EXP_MESH_TANGENT ) != 0 )
				{
					options->exportType &= ~EXP_MESH_TANGENT;
					CheckDlgButton( hWnd, IDC_CHECK_TANGENT, BST_UNCHECKED );
				}
				if ( ( options->exportType & EXP_MESH_TEXCOORD2 ) != 0 )
				{
					options->exportType &= ~EXP_MESH_TEXCOORD2;
					CheckDlgButton( hWnd, IDC_CHECK_TEXCOORD2, BST_UNCHECKED );
				}
			}
			break;
		case IDC_CHECK_TEXCOORD2:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_TEXCOORD2;
				if ( ( options->exportType & EXP_MESH_TEXCOORD ) == 0 )
				{
					options->exportType |= EXP_MESH_TEXCOORD;
					CheckDlgButton( hWnd, IDC_CHECK_TEXCOORD, BST_CHECKED );
				}
			}
			else
			{
				options->exportType &= ~EXP_MESH_TEXCOORD2;
			}
			break;
		case IDC_CHECK_COLOR:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_VERTEX_COLOR;
			}
			else
			{
				options->exportType &= ~EXP_MESH_VERTEX_COLOR;
			}
			break;
		case IDC_CHECK_LOD:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_LOD;
				EnableWindow( GetDlgItem( hWnd, IDC_STATIC_LODSCALE ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_STATIC_MAXERROR ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT_LEVELSCALE ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT_MAXERROR ), TRUE );
			}
			else
			{
				options->exportType &= ~EXP_MESH_LOD;
				EnableWindow( GetDlgItem( hWnd, IDC_STATIC_LODSCALE ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_STATIC_MAXERROR ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT_LEVELSCALE ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT_MAXERROR ), FALSE );
			}
			break;
		case IDC_CHECK_COMPRESS_POS:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_COMPRESS_POS;
			}
			else
			{
				options->exportType &= ~EXP_MESH_COMPRESS_POS;
			}
			break;
		case IDC_CHECK_COMPRESS_NORMAL:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_COMPRESS_NORMAL;
			}
			else
			{
				options->exportType &= ~EXP_MESH_COMPRESS_NORMAL;
			}
			break;
		case IDC_CHECK_COMPRESS_TEXCOORD:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_COMPRESS_TEXCOORD;
			}
			else
			{
				options->exportType &= ~EXP_MESH_COMPRESS_TEXCOORD;
			}
			break;
		case IDC_CHECK_COMPRESS_WEIGHT:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_COMPRESS_WEIGHT;
			}
			else
			{
				options->exportType &= ~EXP_MESH_COMPRESS_WEIGHT;
			}
			break;
		case IDC_CHECK_MERGE:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_MERGEBUFFER;
			}
			else
			{
				options->exportType &= ~EXP_MESH_MERGEBUFFER;
			}
			break;
		case IDC_CHECK_PROPERTY:
			if ( IsDlgButtonChecked( hWnd, LOWORD( wParam ) ) )
			{
				options->exportType |= EXP_MESH_PROPERTY;
			}
			else
			{
				options->exportType &= ~EXP_MESH_PROPERTY;
			}
			break;
		}
		break;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK BspDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static ExportOptions* options = NULL;

	switch ( message )
	{
	case WM_INITDIALOG:
		options = (ExportOptions*)lParam;

		EnableTreeViewCheckbox( GetDlgItem( hWnd, IDC_TREE_BSP ) );
		BuildTreeViewForNodes( GetDlgItem( hWnd, IDC_TREE_BSP ), options->meshNodes );
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
void SetPageState( DWORD dwExpType, IRollupWindow* pRollup )
{
	assert( pRollup != NULL );
	if ( ( dwExpType & EXP_SKELETON ) != 0 )
	{
		ShowPage( pRollup, PAGE_SKELETON, TRUE );
	}
	else
	{
		ShowPage( pRollup, PAGE_SKELETON, FALSE );
	}

	if ( ( dwExpType & EXP_ANIMATION ) != 0 )
	{
		ShowPage( pRollup, PAGE_ANIMATION, TRUE );
	}
	else
	{
		ShowPage( pRollup, PAGE_ANIMATION, FALSE );
	}

	if ( ( dwExpType & EXP_MESH ) == 0 )
	{
		ShowPage( pRollup, PAGE_MESH, FALSE );
	}
	else
	{
		ShowPage( pRollup, PAGE_MESH, TRUE );
	}
}

///////////////////////////////////////////////////////////////////////////////
void ShowPage( IRollupWindow* pRollup, int iIndex, BOOL bShow )
{
	assert( pRollup != NULL );
	if ( iIndex >= pRollup->GetNumPanels() )
	{
		return;
	}
	if ( bShow )
	{
		pRollup->Show( iIndex );
	}
	else
	{
		pRollup->SetPanelOpen( iIndex, FALSE );
		pRollup->Hide( iIndex );
	}
}

///////////////////////////////////////////////////////////////////////////////
void BuildTreeViewForNodes( HWND hTreeView, vector<INode*>* pVecNode )
{
	assert( hTreeView != NULL );
	assert( pVecNode != NULL );

	map<INode*, HTREEITEM> mapItem;

	for ( vector<INode*>::iterator iter = pVecNode->begin();
		iter != pVecNode->end();
		++iter )
	{
		INode* pNode = *iter;
		assert( pNode != NULL );

		HTREEITEM hItemParent = NULL;
		INode* pParent = pNode->GetParentNode();
		map<INode*, HTREEITEM>::iterator iterFound = mapItem.find( pParent );
		if ( iterFound != mapItem.end() )
		{
			hItemParent = (*iterFound).second;
		}

		TVINSERTSTRUCT is;
		is.hParent = ( NULL == hItemParent ) ? TVI_ROOT : hItemParent;
		is.hInsertAfter = TVI_SORT;
		is.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
		is.item.pszText = pNode->GetName();
		is.item.state = TVIS_EXPANDED | INDEXTOSTATEIMAGEMASK(1);
		is.item.stateMask = TVIS_EXPANDED | TVIS_STATEIMAGEMASK;
		is.item.lParam = (LPARAM)pNode;
		HTREEITEM hItem = TreeView_InsertItem( hTreeView, &is );
		mapItem.insert( make_pair( pNode, hItem ) );

		bool dummy = false;
		ObjectState os = pNode->EvalWorldState( 0 );
		if ( os.obj == NULL || os.obj->ClassID() == Class_ID( DUMMY_CLASS_ID, 0 ) )
		{
			dummy = true;
		}
		//如果子节点check保证所有祖先都check
		if ( !pNode->IsHidden() && !dummy )
		{
			SetTreeAncestorCheck( hTreeView, hItem );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void SetTreeAncestorCheck( HWND hTreeView, HTREEITEM hItem )
{
	assert( hTreeView != NULL );
	assert( hItem != NULL );

	while ( hItem != NULL && hItem != TVI_ROOT )
	{
		TreeView_SetCheckState( hTreeView, hItem, TRUE );
		hItem = TreeView_GetParent( hTreeView, hItem );
	}
}

///////////////////////////////////////////////////////////////////////////////
void SetTreeCheckState( HWND hTreeView, HTREEITEM hItem, BOOL bCheck )
{
	assert( hTreeView != NULL );
	assert( hItem != NULL );

	hItem = TreeView_GetChild( hTreeView, hItem );
	while ( hItem != NULL )
	{
		TreeView_SetCheckState( hTreeView, hItem, bCheck );

		SetTreeCheckState( hTreeView, hItem, bCheck );

		hItem = TreeView_GetNextSibling( hTreeView, hItem );
	}
}

///////////////////////////////////////////////////////////////////////////////
void GetSelectedNodeInTreeView( HWND hTreeView, vector<INode*>* pVecNode )
{
	assert( hTreeView != NULL );
	assert( pVecNode != NULL );
	pVecNode->clear();

	HTREEITEM hItem = TreeView_GetChild( hTreeView, TVI_ROOT );
	while ( hItem != NULL )
	{
		GetTreeNode( hTreeView, hItem, pVecNode );
		hItem = TreeView_GetNextSibling( hTreeView, hItem );
	}
}

///////////////////////////////////////////////////////////////////////////////
void GetTreeNode( HWND hTreeView, HTREEITEM hItem, vector<INode*>* pVecNode )
{
	assert( hTreeView != NULL );
	assert( hItem != NULL );
	assert( pVecNode != NULL );

	if ( TreeView_GetCheckState( hTreeView, hItem ) )
	{
		TVITEMEX item;
		item.mask = TVIF_HANDLE;
		item.hItem = hItem;
		if ( TreeView_GetItem( hTreeView, &item ) )
		{
			INode* pNode = (INode*)(item.lParam);
			//INode* pNode =(INode*)TreeView_GetItemState( hTreeView, hItem, TVIF_PARAM );
			assert( pNode != NULL );
			pVecNode->push_back( pNode );
		}
	}

	hItem = TreeView_GetChild( hTreeView, hItem );
	while ( hItem != NULL )
	{
		GetTreeNode( hTreeView, hItem, pVecNode );
		hItem = TreeView_GetNextSibling( hTreeView, hItem );
	}
}