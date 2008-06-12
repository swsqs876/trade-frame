#include "StdAfx.h"
#include "BasketTradeModel.h"

#include "DbValueStream.h"
#include "CommonDatabaseFunctions.h"

#include <ostream>

//
// CBasketTradeModel
//

CBasketTradeModel::CBasketTradeModel( CProviderInterface *pDataProvider, CProviderInterface *pExecutionProvider )
: m_pDataProvider( pDataProvider ), m_pExecutionProvider( pExecutionProvider )
{
}

CBasketTradeModel::~CBasketTradeModel(void) {
  mapBasketSymbols_t::iterator iter;
  for ( iter = m_mapBasketSymbols.begin(); iter != m_mapBasketSymbols.end(); ++iter ) {
    // todo: need to remove any events attached to this object as well
    delete iter->second;
  }
  m_mapBasketSymbols.clear();
}

void CBasketTradeModel::AddSymbol(const std::string &sSymbolName, const std::string &sPath, const std::string &sStrategy) {
  mapBasketSymbols_t::iterator iter;
  iter = m_mapBasketSymbols.find( sSymbolName );
  if ( m_mapBasketSymbols.end() == iter ) {
    m_mapBasketSymbols.insert( pairBasketSymbolsEntry_t( 
      sSymbolName, new CBasketTradeSymbolInfo( sSymbolName, sPath, sStrategy, m_pExecutionProvider ) ) );
    std::cout << "Basket add for " << sSymbolName << " successful." << std::endl;
  }
  else {
    std::cout << "Basket add for " << sSymbolName << " failed, symbol already exists." << std::endl;
  }
}

void CBasketTradeModel::Prepare( ptime dtTradeDate, double dblFunds, bool bRTHOnly ) {
  try {
    mapBasketSymbols_t::iterator iter; 
    double dblFundsPerSymbol = dblFunds / m_mapBasketSymbols.size();
    double dblCostForEntry = 0;
    for( iter = m_mapBasketSymbols.begin(); iter != m_mapBasketSymbols.end(); ++iter ) {
      iter->second->CalculateTrade( dtTradeDate, dblFundsPerSymbol, bRTHOnly );
      dblCostForEntry += iter->second->GetProposedEntryCost();
      m_pDataProvider->AddTradeHandler( iter->second->GetSymbolName(), MakeDelegate( iter->second, &CBasketTradeSymbolInfo::HandleTrade ) );
      m_pDataProvider->AddOnOpenHandler( iter->second->GetSymbolName(), MakeDelegate( iter->second, &CBasketTradeSymbolInfo::HandleOpen ) );
    }
    std::cout << "Total Cost of Entry: " << dblCostForEntry << std::endl;
  }
  catch (...) {
    std::cout << "error somewhere" << std::endl;
  }
}

void CBasketTradeModel::WriteBasketToDatabase() {

  CDbValueStream strm;
  std::ostream out(&strm);
  //streambuf *pOldBuf = out.rdbuf();
  //out.rdbuf( &strm );
  unsigned long key = 0;

  strm.Truncate();
  for( mapBasketSymbols_t::iterator iter = m_mapBasketSymbols.begin(); iter != m_mapBasketSymbols.end(); ++iter ) {
    //out.clear();  // one of these calls sync to clear the buffer
    out.flush();
    iter->second->StreamSymbolInfo( &out );
    strm.Save( &key, sizeof( key ) );
    ++key;
  }

  //out.rdbuf( pOldBuf );
}

void CBasketTradeModel::ReadBasketFromDatabase() {
  std::stringstream in;
  CBasketTradeSymbolInfo *pBasket = new CBasketTradeSymbolInfo( &in, m_pExecutionProvider );
}