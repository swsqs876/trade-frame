/************************************************************************
 * Copyright(c) 2015, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

// started December 13, 2015, 2:13 PM

#include "PanelChartsOps.h"

void TreeItemGroupCommon::BuildContextMenu( wxMenu* pMenu ) {
  assert( 0 != pMenu );
  pMenu->Append( MIAddInstrument, "Add &Instrument" );
  pMenu->Bind( wxEVT_COMMAND_MENU_SELECTED, &TreeItemGroupCommon::HandleAddInstrument, this, MIAddInstrument );
  pMenu->Append( MIAddPortfolio, "Add P&ortfolio" );
  pMenu->Bind( wxEVT_COMMAND_MENU_SELECTED, &TreeItemGroupCommon::HandleAddPortfolio, this, MIAddPortfolio );
  pMenu->Append( MIAddPosition, "Add Pos&ition" );
  pMenu->Bind( wxEVT_COMMAND_MENU_SELECTED, &TreeItemGroupCommon::HandleAddPosition, this, MIAddPosition );
  pMenu->Append( MIAddGroup, "Add &Group" );
  pMenu->Bind( wxEVT_COMMAND_MENU_SELECTED, &TreeItemGroupCommon::HandleAddGroup, this, MIAddGroup );
  pMenu->Append( MIRename, "&Rename" );
  pMenu->Bind( wxEVT_COMMAND_MENU_SELECTED, &TreeItemBase::HandleRename, this, MIRename );
}

void TreeItemGroupCommon::HandleAddInstrument( wxCommandEvent& event ) { 
  TreeItemInstrument* p = AddTreeItem<TreeItemInstrument>( "Instrument", IdInstrument, m_resources );
  p->Rename();
}

void TreeItemGroupCommon::HandleAddPortfolio( wxCommandEvent& event ) { 
  TreeItemPortfolio* p = AddTreeItem<TreeItemPortfolio>( "Portfolio", IdPortfolio, m_resources );
  p->Rename();
}

void TreeItemGroupCommon::HandleAddPosition( wxCommandEvent& event ) { 
  TreeItemPosition* p = AddTreeItem<TreeItemPosition>( "Position", IdPosition, m_resources );
  p->Rename();
}

void TreeItemGroupCommon::HandleAddGroup( wxCommandEvent& event ) { 
  TreeItemGroup* p = AddTreeItem<TreeItemGroup>( "Group", IdGroup, m_resources );
  p->Rename();
}

// ================

void TreeItemRoot::ShowContextMenu( void ) {
  wxMenu* pMenu = new wxMenu();  // menu does not get deleted, so may need to reclaim afterwards.  put into a list somewhere?
  TreeItemGroupCommon::BuildContextMenu( pMenu );
  m_baseResources.signalPopupMenu( pMenu );
}

// ================

void TreeItemGroup::ShowContextMenu( void ) {
  wxMenu* pMenu = new wxMenu();  // menu does not get deleted, so may need to reclaim afterwards.  put into a list somewhere?
  TreeItemGroupCommon::BuildContextMenu( pMenu );
  pMenu->Append( MIDelete, "Delete" );
  pMenu->Bind( wxEVT_COMMAND_MENU_SELECTED, &TreeItemGroup::HandleDelete, this, MIDelete );
  m_baseResources.signalPopupMenu( pMenu );
}

void TreeItemGroup::HandleDelete( wxCommandEvent& event ) {
  std::cout << "TreeItemGroup Delete" << std::endl;
  m_baseResources.signalDelete( this->m_id );
}
