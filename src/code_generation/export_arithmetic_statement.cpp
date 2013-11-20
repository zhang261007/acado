/*
 *    This file is part of ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2013 by Boris Houska, Hans Joachim Ferreau,
 *    Milan Vukov, Rien Quirynen, KU Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC)
 *    under supervision of Moritz Diehl. All rights reserved.
 *
 *    ACADO Toolkit is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    ACADO Toolkit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with ACADO Toolkit; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */



/**
 *    \file src/code_generation/export_arithmetic_statement.cpp
 *    \author Hans Joachim Ferreau, Boris Houska
 *    \date 2010-2011
 */

#include <acado/code_generation/export_arithmetic_statement.hpp>
#include <acado/code_generation/export_variable_internal.hpp>

#include <iomanip>


BEGIN_NAMESPACE_ACADO

using namespace std;

//
// PUBLIC MEMBER FUNCTIONS:
//

ExportArithmeticStatement::ExportArithmeticStatement( )
{
	op0 = ESO_UNDEFINED;
	op1 = ESO_UNDEFINED;
	op2 = ESO_UNDEFINED;
}

ExportArithmeticStatement::ExportArithmeticStatement(	const ExportVariable& _lhs,
														ExportStatementOperator _op0,
														const ExportVariable& _rhs1,
														ExportStatementOperator _op1,
														const ExportVariable& _rhs2,
														ExportStatementOperator _op2,
														const ExportVariable& _rhs3
														) : ExportStatement( )
{
	ASSERT( ( _op0 == ESO_UNDEFINED ) || ( _op0 == ESO_ASSIGN ) || ( _op0 == ESO_ADD_ASSIGN ) || ( _op0 == ESO_SUBTRACT_ASSIGN ) );
	ASSERT( ( _op2 == ESO_UNDEFINED ) || ( _op2 == ESO_ADD ) || ( _op2 == ESO_SUBTRACT ) );

	lhs = _lhs;
	rhs1 = _rhs1;
	rhs2 = _rhs2;
	rhs3 = _rhs3;

	op0  = _op0;
	op1  = _op1;
	op2  = _op2;
}

ExportArithmeticStatement::~ExportArithmeticStatement( )
{}

ExportStatement* ExportArithmeticStatement::clone( ) const
{
	return new ExportArithmeticStatement(*this);
}


uint ExportArithmeticStatement::getNumRows( ) const
{
	if ( rhs1.isNull() )
		return 0;
	else
	{
		if ( op1 != ESO_MULTIPLY_TRANSPOSE )
			return rhs1->getNumRows( );
		else
			return rhs1->getNumCols( );
	}
}


uint ExportArithmeticStatement::getNumCols( ) const
{
	if ( rhs1.isNull() )
		return 0;
	else
	{
		if ( rhs2.isNull() )
			return rhs1->getNumCols( );
		else
			return rhs2->getNumCols( );
	}
}


returnValue ExportArithmeticStatement::exportDataDeclaration(	std::ostream& stream,
																const std::string& _realString,
																const std::string& _intString,
																int _precision
																) const
{
	return SUCCESSFUL_RETURN;
}


returnValue ExportArithmeticStatement::exportCode(	std::ostream& stream,
													const std::string& _realString,
													const std::string& _intString,
													int _precision
													) const
{
	if (lhs->isGiven() == true && lhs->getDim() > 0)
	{
		LOG( LVL_ERROR ) << "Left hand side ('" << lhs.getFullName() << "') of an arithmetic "
							"expression is given." << endl;
		return ACADOERROR(RET_INVALID_ARGUMENTS);
	}

	if (memAllocator == 0)
		return ACADOERRORTEXT(RET_INVALID_ARGUMENTS, "Memory allocator is not defined.");
	
	switch( op1 )
	{
		case ESO_ADD:
			return exportCodeAddSubtract(stream, "+", _realString, _intString, _precision);

		case ESO_SUBTRACT:
			return exportCodeAddSubtract(stream, "-", _realString, _intString, _precision);
			
		case ESO_ADD_ASSIGN:
			return exportCodeAssign( stream,"+=",_realString,_intString,_precision );

		case ESO_SUBTRACT_ASSIGN:
			return exportCodeAssign( stream,"-=",_realString,_intString,_precision );

		case ESO_MULTIPLY:
			return exportCodeMultiply( stream,false,_realString,_intString,_precision );
			
		case ESO_MULTIPLY_TRANSPOSE:
			return exportCodeMultiply( stream,true,_realString,_intString,_precision );

		case ESO_ASSIGN:
			return exportCodeAssign( stream,"=",_realString,_intString,_precision );

		default:
			return ACADOERROR( RET_UNKNOWN_BUG );
	}
	
	return ACADOERROR( RET_UNKNOWN_BUG );
}

//
// PROTECTED MEMBER FUNCTIONS:
//

returnValue ExportArithmeticStatement::exportCodeAddSubtract(	std::ostream& stream,
																const std::string& _sign,
																const std::string& _realString,
																const std::string& _intString,
																int _precision
																) const
{
//	if ( ( rhs1.isNull() ) || ( rhs2.isNull() ) || ( !rhs3.isNull() ) )
//		return ACADOERROR( RET_UNABLE_TO_EXPORT_STATEMENT );

	if (rhs1.getDim() == 0)
		return SUCCESSFUL_RETURN;

	if ( ( rhs1->getNumRows() != rhs2->getNumRows() ) || 
		 ( rhs1->getNumCols() != rhs2->getNumCols() ) )
		return ACADOERROR( RET_VECTOR_DIMENSION_MISMATCH );
	
	if ( !lhs.isNull() )
	{
		if ( ( rhs1->getNumRows() != lhs->getNumRows() ) || 
		     ( rhs1->getNumCols() != lhs->getNumCols() ) )
		{
			LOG( LVL_DEBUG )
					<< "lhs name is " << lhs.getName() <<
					", size: " << lhs.getNumRows() << " x " << lhs.getNumCols() << endl
					<< "rhs1 name is " << rhs1.getName() <<
					", size: " << rhs1.getNumRows() << " x " << rhs1.getNumCols() << endl;

			return ACADOERROR( RET_VECTOR_DIMENSION_MISMATCH );
		}
	}
	
	std::string assignString;
	if ( getAssignString( assignString ) != SUCCESSFUL_RETURN )
		return ACADOERROR( RET_UNABLE_TO_EXPORT_STATEMENT );
	
	//
	// Rough approximation of flops needed for matrix multiplication
	//
	unsigned numberOfFlops = lhs->getNumRows() * lhs->getNumCols();

	//
	// Optimization can be performed only if both matrices are not given.
	// Currently, optimizations cannot be performed on hard-coded matrices.
	//
	int optimizationsAllowed =
			( rhs1->isGiven() == false ) && ( rhs2->isGiven() == false );

	if ((numberOfFlops < 4096) || (optimizationsAllowed == 0))
	{
		for( uint i=0; i<getNumRows( ); ++i )
			for( uint j=0; j<getNumCols( ); ++j )
			{
				if ( ( op0 != ESO_ASSIGN ) &&
						( rhs1->isGiven(i,j) == true ) && ( rhs2->isGiven(i,j) == true ) )
				{
					// check for zero value in case of "+=" or "-="
					if ( ( op1 == ESO_ADD ) && ( acadoIsZero(rhs1(i, j) + rhs2(i, j)) == true ) )
						continue;

					if ( ( op1 == ESO_SUBTRACT ) && ( acadoIsZero( rhs1(i, j) - rhs2(i, j)) == true ) )
						continue;
				}

				if ( !lhs.isNull() )
					stream << lhs.get(i, j) << " " << assignString << " ";

				if ( rhs1->isZero(i, j) == false )
				{
					stream << rhs1->get(i, j);
					if ( rhs2->isZero(i,j) == false )
						stream << _sign << " " << rhs2->get(i, j) << ";\n";
					else
						stream << ";" << endl;
				}
				else
				{
					if (rhs2->isZero(i, j) == false)
						stream << _sign << " " << rhs2->get(i, j) << ";\n";
					else
						stream << "0.0;\n";
				}
			}
	}
	else if ( numberOfFlops < 32768 )
	{
		ExportIndex ii;
		memAllocator->acquire( ii );

		stream << "for (" << ii.getName() << " = 0; ";
		stream << ii.getName() << " < " << getNumRows() << "; ";
		stream << "++" << ii.getName() << ")\n{\n";

		for(unsigned j = 0; j < getNumCols( ); ++j)
		{
			stream << lhs->get(ii, j) << " " << assignString;
			stream << _sign << " " << rhs2->get(ii, j) << ";\n";
		}

		stream << "\n{\n";

		memAllocator->release( ii );
	}
	else
	{
		ExportIndex ii, jj;
		memAllocator->acquire( ii );
		memAllocator->acquire( jj );

		stream	<< "for (" << ii.getName() << " = 0; "
				<< ii.getName() << " < " << getNumRows() <<"; "
				<< "++" << ii.getName() << ")\n{\n";

		stream	<< "for (" << jj.getName() << " = 0; "
				<< jj.getName() << " < " << getNumCols() <<"; "
				<< "++" << jj.getName() << ")\n{\n";

		stream	<< lhs->get(ii, jj) << " " <<  assignString
				<< _sign << " " << rhs2->get(ii, jj) << ";\n";

		stream	<< "\n}\n"
				<< "\n}\n";

		memAllocator->release( ii );
		memAllocator->release( jj );
	}

	return SUCCESSFUL_RETURN;
}

returnValue ExportArithmeticStatement::exportCodeMultiply(	std::ostream& stream,
															bool transposeRhs1,
															const std::string& _realString,
															const std::string& _intString,
															int _precision
															) const
{
//	if ( ( lhs.isNull() ) || ( rhs1.isNull() ) || ( rhs2.isNull() ) )
//		return ACADOERROR( RET_UNABLE_TO_EXPORT_STATEMENT );

	if (lhs.getDim() == 0 || rhs1.getDim() == 0 || rhs2.getDim() == 0)
		return SUCCESSFUL_RETURN;

	std::string assignString;
	if ( getAssignString( assignString ) != SUCCESSFUL_RETURN )
		return ACADOERROR( RET_UNABLE_TO_EXPORT_STATEMENT );

	uint nRowsRhs1;
	uint nColsRhs1;

	if ( transposeRhs1 == false )
	{
		nRowsRhs1 = rhs1->getNumRows( );
		nColsRhs1 = rhs1->getNumCols( );
	}
	else
	{
		nRowsRhs1 = rhs1->getNumCols( );
		nColsRhs1 = rhs1->getNumRows( );
	}

	if ( ( nColsRhs1 != rhs2->getNumRows( ) ) ||
			( nRowsRhs1 != lhs->getNumRows( ) ) ||
			( rhs2->getNumCols( ) != lhs->getNumCols( ) ) )
	return ACADOERROR( RET_VECTOR_DIMENSION_MISMATCH );

	char sign[2] = "+";

	if ( op2 != ESO_UNDEFINED )
	{
		if ( ( rhs3->getNumRows( ) != lhs->getNumRows( ) ) ||
				( rhs3->getNumCols( ) != lhs->getNumCols( ) ) )
		return ACADOERROR( RET_VECTOR_DIMENSION_MISMATCH );

		if ( op2 == ESO_SUBTRACT )
		sign[0] = '-';
	}

	bool allZero;

	ExportIndex ii, iiRhs1;
	ExportIndex jj, jjRhs1;
	ExportIndex kk, kkRhs1;

	//
	// Rough approximation of flops needed for matrix multiplication
	//
	unsigned numberOfFlops = nRowsRhs1 * rhs2->getNumRows( ) * rhs2->getNumCols();

	//
	// Optimization can be performed only if both matrices are not given.
	// Currently, optimizations cannot be performed on hard-coded matrices.
	//
	int optimizationsAllowed =
			( rhs1->isGiven() == false ) && ( rhs2->isGiven() == false );

	//
	// Depending on the flops count different export strategies are performed
	//
	if ( ( numberOfFlops < 4096 ) || ( optimizationsAllowed == 0) )
	{
		//
		// Unroll all loops
		//

		for(uint i = 0; i < getNumRows( ); ++i)
		{
			ii = i;

			for(uint j = 0; j < getNumCols( ); ++j)
			{
				allZero = true;

				stream << lhs->get(ii,j) <<  " " << assignString;

				for(uint k = 0; k < nColsRhs1; ++k)
				{
					kk = k;
					if ( transposeRhs1 == false )
					{
						iiRhs1 = ii;
						kkRhs1 = kk;
					}
					else
					{
						iiRhs1 = kk;
						kkRhs1 = ii;
					}

					if ( ( rhs1->isZero(iiRhs1,kkRhs1) == false ) &&
							( rhs2->isZero(kk,j) == false ) )
					{
						allZero = false;

						if ( rhs1->isOne(iiRhs1,kkRhs1) == false )
						{
							stream << sign << " " << rhs1->get(iiRhs1,kkRhs1);

							if ( rhs2->isOne(kk,j) == false )
								stream << "*" << rhs2->get(kk, j);
						}
						else
						{
							if ( rhs2->isOne(kk,j) == false )
								stream << " " << sign << rhs2->get(kk,j);
							else
								stream << " " << sign << " 1.0";
						}
					}
				}

				if ( ( op2 == ESO_ADD ) || ( op2 == ESO_SUBTRACT ) )
					stream << " + " << rhs3->get(ii, j) << ";\n";

				if ( op2 == ESO_UNDEFINED )
				{
					if ( allZero == true )
						stream << " 0.0;\n";
					else
						stream << ";\n";
				}
			}
		}
	}
	else if ( numberOfFlops < 32768 )
	{
		//
		// Unroll two inner loops
		//

		memAllocator->acquire( ii );

		stream << "for (" << ii.getName() << " = 0; ";
		stream << ii.getName() << " < " << getNumRows() <<"; ";
		stream << "++" << ii.getName() << ")\n{\n";

		for(uint j = 0; j < getNumCols( ); ++j)
		{
			allZero = true;

			stream << lhs->get(ii,j) << " " << assignString;

			for(uint k = 0; k < nColsRhs1; ++k)
			{
				kk = k;
				if ( transposeRhs1 == false )
				{
					iiRhs1 = ii;
					kkRhs1 = kk;
				}
				else
				{
					iiRhs1 = kk;
					kkRhs1 = ii;
				}

				if ( ( rhs1->isZero(iiRhs1,kkRhs1) == false ) &&
					( rhs2->isZero(kk,j) == false ) )
				{
					allZero = false;

					if ( rhs1->isOne(iiRhs1,kkRhs1) == false )
					{
						stream << sign << " " << rhs1->get(iiRhs1,kkRhs1);
						if ( rhs2->isOne(kk,j) == false )
							stream << "*" << rhs2->get(kk, j);
					}
					else
					{
						if ( rhs2->isOne(kk,j) == false )
							stream << " " <<  sign << " " << rhs2->get(kk,j);
						else
							stream << " " << sign << " 1.0";
					}
				}
			}

			if ( ( op2 == ESO_ADD ) || ( op2 == ESO_SUBTRACT ) )
				stream << " + " << rhs3->get(ii,j) << ";\n";

			if ( op2 == ESO_UNDEFINED )
			{
				if ( allZero == true )
					stream << " 0.0;\n";
				else
					stream << ";\n";
			}
		}
		stream << "\n}\n";

		memAllocator->release( ii );
	}
	else
	{
		//
		// Keep rolled first two outer loops
		//

		memAllocator->acquire( ii );
		memAllocator->acquire( jj );

		// First loop
		stream << "for (" << ii.getName() << " = 0; ";
		stream << ii.getName() << " < " << getNumRows() <<"; ";
		stream << "++" << ii.getName() << ")\n{\n";

		// Second loop
		stream << "for (" << jj.getName() << " = 0; ";
		stream << jj.getName() << " < " << getNumCols() <<"; ";
		stream << "++" << jj.getName() << ")\n{\n";

		allZero = true;

		stream << lhs->get(ii, jj) << " " << assignString;

		for(uint k = 0; k < nColsRhs1; ++k)
		{
			kk = k;
			if ( transposeRhs1 == false )
			{
				iiRhs1 = ii;
				kkRhs1 = kk;
			}
			else
			{
				iiRhs1 = kk;
				kkRhs1 = ii;
			}

			if ( ( rhs1->isZero(iiRhs1,kkRhs1) == false ) &&
					( rhs2->isZero(kk,jj) == false ) )
			{
				allZero = false;

				if ( rhs1->isOne(iiRhs1,kkRhs1) == false )
				{
					stream << " " << sign << " " << rhs1->get(iiRhs1, kkRhs1);

					if ( rhs2->isOne(kk,jj) == false )
						stream << "*" << rhs2->get(kk, jj);
				}
				else
				{
					if ( rhs2->isOne(kk,jj) == false )
						stream << " " << sign << " " << rhs2->get(kk, jj);
					else
						stream << " " << sign << " 1.0";
				}
			}
		}

		if ( ( op2 == ESO_ADD ) || ( op2 == ESO_SUBTRACT ) )
			stream << " + " << rhs3->get(ii,jj) << ";\n";

		if ( op2 == ESO_UNDEFINED )
		{
			if ( allZero == true )
				stream << " 0.0;\n";
			else
				stream << ";\n";
		}

		stream << "\n}\n";
		stream << "\n}\n";

		memAllocator->release( ii );
		memAllocator->release( jj );
	}

	return SUCCESSFUL_RETURN;
}


returnValue ExportArithmeticStatement::exportCodeAssign(	std::ostream& stream,
															const std::string& _op,
															const std::string& _realString,
															const std::string& _intString,
															int _precision
															) const
{
	if (lhs.getDim() == 0 || rhs1.getDim() == 0 || rhs2.getDim() == 0)
		return SUCCESSFUL_RETURN;

	if ( ( rhs1.getNumRows( ) != lhs.getNumRows( ) ) ||
		 ( rhs1.getNumCols( ) != lhs.getNumCols( ) ) )
	{
		LOG( LVL_DEBUG ) << "lhs name is " << lhs.getName()
				<< ", size: " << lhs.getNumRows() << " x " << lhs.getNumCols()
				<< "rhs1 name is " << rhs1.getName()
				<< ", size: " << rhs1.getNumRows() << " x " << rhs1.getNumCols() << endl;

		return ACADOERROR( RET_VECTOR_DIMENSION_MISMATCH );
	}

	stream << setprecision( _precision );

	unsigned numOps = lhs.getNumRows() * lhs.getNumCols();

	if (	lhs.isSubMatrix() == false && lhs.getDim() > 1 &&
			rhs1.isGiven() == true && rhs1.getGivenMatrix().isZero() == true)
	{
		stream 	<< "{ int lCopy; for (lCopy = 0; lCopy < "<< lhs.getDim() << "; lCopy++) "
				<< lhs.getFullName() << "[ lCopy ] = 0.0; }" << endl;
	}
	else if ((numOps < 128) || (rhs1.isGiven() == true))
	{
		for(unsigned i = 0; i < lhs.getNumRows( ); ++i)
			for(unsigned j = 0; j < lhs.getNumCols( ); ++j)
				if ( ( _op == (std::string)"=" ) || ( rhs1.isZero(i,j) == false ) )
				{
					stream << lhs->get(i, j) << " " << _op << " ";
					if (rhs1->isGiven() == true)
					{
						stream << scientific << rhs1(i, j) << ";\n";
					}
					else
					{
						stream << rhs1->get(i, j) << ";\n";
					}
				}
	}
	else
	{
		ExportIndex ii, jj;

		if (lhs.getNumCols() == 1 || lhs.getNumRows() == 1)
		{
			memAllocator->acquire( ii );

			stream << "for (" << ii.get() << " = 0; " << ii.get() << " < ";

			if (lhs->getNumCols() == 1)
			{
				stream << lhs->getNumRows() << "; ++" << ii.getName() << ")" << endl
						<< lhs.get(ii, 0) << " " << _op << " " << rhs1.get(ii, 0)
						<< ";" << endl << endl;
			}
			else
			{
				stream << lhs.getNumCols() << "; ++" << ii.getName() << ")" << endl;
				stream << lhs.get(0, ii) << " " << _op << " " << rhs1.get(0, ii)
						<< ";" << endl << endl;
			}

			memAllocator->release( ii );
		}
		else
		{
			memAllocator->acquire( ii );
			memAllocator->acquire( jj );

			stream << "for (" << ii.getName() << " = 0;" << ii.getName() << " < "
					<< lhs->getNumRows() << "; ++" << ii.getName() << ")" << endl;

			stream << "for (" << jj.getName() << " = 0;" << jj.getName() << " < "
					<< lhs->getNumCols() << "; ++" << jj.getName() << ")" << endl;

			stream << lhs->get(ii, jj) << " " << _op << " " << rhs1->get(ii, jj) << ";" << endl;

			memAllocator->release( ii );
			memAllocator->release( jj );
		}
	}

	return SUCCESSFUL_RETURN;
}


returnValue ExportArithmeticStatement::getAssignString(	std::string& _assignString
														) const
{
	switch ( op0 )
	{
		case ESO_ASSIGN:
			_assignString = "=";
			return SUCCESSFUL_RETURN;
		
		case ESO_ADD_ASSIGN:
			_assignString = "+=";
			return SUCCESSFUL_RETURN;
			
		case ESO_SUBTRACT_ASSIGN:
			_assignString = "-=";
			return SUCCESSFUL_RETURN;
			
		default:
			return RET_UNABLE_TO_EXPORT_STATEMENT;
	}
}

ExportArithmeticStatement& ExportArithmeticStatement::allocate( MemoryAllocatorPtr allocator )
{
	memAllocator = allocator;

	return *this;
}

CLOSE_NAMESPACE_ACADO

// end of file.
