/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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

#include "StdAfx.h"
#include "InstrumentFile.h"

#include "TradingEnumerations.h"

#include <string>
#include <stdexcept>
#include <cassert>

// todo 2008/06/06 change record retrieval to use stack structures rather than dynamic allocation

CInstrumentFile::CInstrumentFile(void) 
: pRecord( NULL ), 
  m_pdbSymbols( NULL ), 
  m_pdbIxSymbols_Market( NULL ), m_pdbcIxSymbols_Market( NULL ),
  m_pdbIxSymbols_Underlying( NULL ), m_pdbcIxSymbols_Underlying( NULL ) 
{
}

CInstrumentFile::~CInstrumentFile(void) {
}

void CInstrumentFile::OpenIQFSymbols() {

  CBerkeleyDBEnvManagerSingleton EnvMgr;
  DbEnv *pDbEnv = EnvMgr.Instance().GetDbEnv();

  // open/create main symbol table
  m_pdbSymbols = new Db( pDbEnv, 0 );
  m_pdbSymbols->open( NULL, EnvMgr.Instance().GetBDBFileName(), "IQFSymbols", DB_BTREE, DB_CREATE, 0 );

  // open/create the market index
  m_pdbIxSymbols_Market = new Db( pDbEnv, 0 );
  m_pdbIxSymbols_Market->set_flags( DB_DUPSORT );
  m_pdbIxSymbols_Market->open( NULL, EnvMgr.Instance().GetBDBFileName(), "IxIQFSymbols_Market", DB_BTREE, DB_CREATE, 0 );

  // associate the index with the main table
  m_pdbSymbols->associate( NULL, m_pdbIxSymbols_Market, &CInstrumentFile::GetMarketName, 0 );

  // open/create the underlying index
  m_pdbIxSymbols_Underlying = new Db( pDbEnv, 0 );
  m_pdbIxSymbols_Underlying->set_flags( DB_DUPSORT );
  m_pdbIxSymbols_Underlying->open( NULL, EnvMgr.Instance().GetBDBFileName(), "IxIQFSymbols_Underlying", DB_BTREE, DB_CREATE, 0 );

  // associate the index with the main table
  m_pdbSymbols->associate( NULL, m_pdbIxSymbols_Underlying, &CInstrumentFile::GetUnderlyingName, 0 );
  
}

void CInstrumentFile::CloseIQFSymbols() {
  m_pdbIxSymbols_Underlying->close(0);
  m_pdbIxSymbols_Market->close(0);
  m_pdbSymbols->close(0);
}

CInstrument::pInstrument_t CInstrumentFile::CreateInstrumentFromIQFeed(const std::string &sIQFeedSymbolName, const std::string &sAlternateSymbolName ) {
  // look up IQFeed based table with IQFeedSymbolName, but return instrument with Alternate Symbol name
  Dbt k;
  k.set_data( (void*) sIQFeedSymbolName.c_str() );
  k.set_size( sIQFeedSymbolName.size() );
  structSymbolRecord rec;
  Dbt v;
  v.set_flags( DB_DBT_USERMEM );
  v.set_ulen( sizeof( structSymbolRecord ) );
  v.set_size( sizeof( structSymbolRecord ) );
  v.set_data( &rec );
  int ret;
  try {
    //ret = m_pdbSymbols->exists( 0, &k, 0 );
    ret = m_pdbSymbols->get( 0, &k ,&v, 0 );
  }
  catch ( DbException  e ) {
    std::cout << "CInstrumentFile::CreateInstrumentFromIQFeed " << e.what() << std::endl;
  }
  catch ( ... ) {
    std::cout << "CInstrumentFile::CreateInstrumentFromIQFeed bummer error" << std::endl;
  }
  if ( DB_NOTFOUND == ret ) throw std::out_of_range( "CInstrumentFile::CreateInstrumentFromIQFeed symbol not found" );
  if ( 0 != ret ) throw std::runtime_error( "CInstrumentFile::CreateInstrumentFromIQFeed had bad error" );
  //pRecord = (structSymbolRecord*) v.get_data();
  //UnPackBoolean( rec.ucBits1 );
  std::string sExchange( rec.line + rec.ix[2], rec.cnt[2] );

  switch ( rec.eInstrumentType ) {
    case InstrumentType::Stock: {
        CInstrument::pInstrument_t pInstrument( new CInstrument( sAlternateSymbolName, sExchange, InstrumentType::Stock ) );
        return pInstrument;
      }
      break;
    case InstrumentType::Option: {
        const char *p = rec.line + rec.ix[1]; 
        const char *e = strchr( p, ' ' );  
        u_int32_t len = e - p;
        std::string sUnderlying( rec.line + rec.ix[1], len );
        CInstrument::pInstrument_t pInstrument( new CInstrument( sAlternateSymbolName, sExchange, 
          (InstrumentType::enumInstrumentTypes) rec.eInstrumentType, 
          rec.nYear, rec.nMonth,
          sUnderlying, (OptionSide::enumOptionSide) rec.nOptionSide, 
          rec.fltStrike ) );
        return pInstrument;
      }
      break;
    case InstrumentType::Future: {
         CInstrument::pInstrument_t pInstrument( new CInstrument( sAlternateSymbolName, sExchange, (InstrumentType::enumInstrumentTypes) rec.eInstrumentType, rec.nYear, rec.nMonth ) );
         return pInstrument;
       }
      break;
    default:
      throw std::out_of_range( "Unknown instrument type" ); 
  }
}

// Sets SubIndex (static function)
int CInstrumentFile::GetMarketName( Db *secondary, const Dbt *pKey, const Dbt *data, Dbt *secKey ) {
  structSymbolRecord *dbIxRecord = (structSymbolRecord *) data->get_data();
  char *p = dbIxRecord->line + dbIxRecord->ix[2];  // get at the 'exchange' string
  unsigned long l = dbIxRecord->cnt[2];  // set the key and its length
  secKey->set_data( p );
  secKey->set_size( l );
  return 0;
}

// Sets SubIndex (static function)
int CInstrumentFile::GetUnderlyingName( Db *secondary, const Dbt *pKey, const Dbt *data, Dbt *secKey ) {
  structSymbolRecord *dbIxRecord = (structSymbolRecord *) data->get_data();
  char *p; 
  u_int32_t len;
  bool bUseIndex = true;
  if ( InstrumentType::Option == dbIxRecord->eInstrumentType ) { // OPRA option
    p = dbIxRecord->line + dbIxRecord->ix[1];  // start of description
    char *e = strchr( p, ' ' );  // find the trailing blank
    len = e - p;
    if ( 0 != len ) bUseIndex = false;
  }
  if ( bUseIndex ) {  // by default, use records symbol
    p = dbIxRecord->line;
    len = dbIxRecord->cnt[0];
  }
  secKey->set_data( p );
  secKey->set_size( len );
  return 0;
}

void CInstrumentFile::SetSearchExchange( const char *szExchange ) {
  m_szSearchKey = szExchange;
  m_lenSearchKey = strlen( szExchange );
  m_dbtKey.set_data( (void*) szExchange );
  m_dbtKey.set_size( m_lenSearchKey );
  int ret = m_pdbIxSymbols_Market->cursor( NULL, &m_pdbcIxSymbols_Market, 0 );
  if ( 0 != ret ) throw std::runtime_error( "CInstrumentFile::SetSearchExchange has problems" );
}

void CInstrumentFile::SetSearchUnderlying( const char *szUnderlying ) {
  m_szSearchKey = szUnderlying;
  m_lenSearchKey = strlen( szUnderlying );
  m_dbtKey.set_data( (void*) szUnderlying );
  m_dbtKey.set_size( m_lenSearchKey );
  int ret = m_pdbIxSymbols_Underlying->cursor( NULL, &m_pdbcIxSymbols_Underlying, 0 );
  if ( 0 != ret ) throw std::runtime_error( "CInstrumentFile::SetSearchUnderlying has problems" );
}

bool CInstrumentFile::RetrieveSymbolRecordByExchange( u_int32_t flags ) {
  // flags =  DB_SET: find first record
  //          DB_NEXT_DUP:  retrieve next record for same exchange
  pRecord = NULL;
  assert( NULL != m_pdbcIxSymbols_Market );
  int result = m_pdbcIxSymbols_Market->get( &m_dbtKey, &m_dbtData, flags );
  if ( 0 == result ) {
    pRecord = (structSymbolRecord *) m_dbtData.get_data();
//    UnPackBoolean( pRecord->ucBits1 );
  }
  return ( 0 == result );
}

bool CInstrumentFile::RetrieveSymbolRecordByUnderlying( u_int32_t flags ) {
  pRecord = NULL;
  assert( NULL != m_pdbcIxSymbols_Underlying );
  int result = m_pdbcIxSymbols_Underlying->get( &m_dbtKey, &m_dbtData, flags );
  if ( 0 == result ) {
    pRecord = (structSymbolRecord *) m_dbtData.get_data();
//    UnPackBoolean( pRecord->ucBits1 );
  }
  return ( 0 == result );
}

void CInstrumentFile::EndSearch( void ) {
}
