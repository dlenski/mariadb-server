/* Copyright (C) 2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef NDBT_CALC_HPP
#define NDBT_CALC_HPP

#include <NDBT_ResultRow.hpp>

/* *************************************************************
 * HugoCalculator
 *
 *  Comon class for the Hugo test suite, provides the functions 
 *  that is used for calculating values to load in to table and 
 *  also knows how to verify a row that's been read from db 
 *
 * ************************************************************/
class HugoCalculator {
public:
  HugoCalculator(const NdbDictionary::Table& tab);
  Int32 calcValue(int record, int attrib, int updates) const;
#if 0
  U_Int32 calcValue(int record, int attrib, int updates) const;
  U_Int64 calcValue(int record, int attrib, int updates) const;
  Int64 calcValue(int record, int attrib, int updates) const;
  float calcValue(int record, int attrib, int updates) const;
  double calcValue(int record, int attrib, int updates) const;
#endif
  const char* calcValue(int record, int attrib, int updates, char* buf) const;

  int verifyRowValues(NDBT_ResultRow* const  pRow) const;
  int getIdValue(NDBT_ResultRow* const pRow) const;
  int getUpdatesValue(NDBT_ResultRow* const pRow) const;
  int isIdCol(int colId) { return m_idCol == colId; };
  int isUpdateCol(int colId){ return m_updatesCol == colId; };
private:
  const NdbDictionary::Table& m_tab;
  int m_idCol;
  int m_updatesCol;
};


#endif




