/**
 * @author Stefan Brandle
 * @date April, 2004 Updated for multi-round play in 2015.
 * CleanPlayer AI
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "CleanPlayerV2.h"
#include "conio.h"

using namespace std;
using namespace conio;

/**
 * @brief Constructor that initializes any inter-round data structures.
 * @param boardSize Indication of the size of the board that is in use.
 */
CleanPlayerV2::CleanPlayerV2( int boardSize )
    :PlayerV2(boardSize)
{
    this->initializeBoard(this->board);
    this->initializeBoard(this->shipBoard);
    for(int row=0; row<boardSize; row++) {
	for(int col=0; col<boardSize; col++) {
	    this->opponentShots[row][col] = 0;
	}
    }
}

/*
 * Private internal function that initializes a MAX_BOARD_SIZE 2D array of char to water.
 */
void CleanPlayerV2::initializeBoard(char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]) {
    for(int row=0; row<boardSize; row++) {
	for(int col=0; col<boardSize; col++) {
	    board[row][col] = WATER;
	}
    }
}
/**
 * Gets the computer's shot choice. This is then returned to the caller.
 * @return int[] shot
 *  Position 0 of the int array should hold the row, position 1 the column.
 */
Message CleanPlayerV2::getMove( ) {
    int shotRow = scanRow;
    int shotCol = scanCol;

    if( board[scanRow][scanCol] == WATER ) {
	// first shot or something like that -- just shoot at existing choice
    }
    else if( board[scanRow][scanCol] == HIT ) {
	getFollowUpShot( shotRow, shotCol );
    } else {
	scan( scanRow, scanCol );
	shotRow = scanRow;
	shotCol = scanCol;
    }
    Message shotMessage( SHOT, shotRow, shotCol, string("Bang"), None, 1 );
    return shotMessage;
}

/**
 * @brief Asks the AI where it wishes to place one of its ships.
 * @param length The length of the ship to be placed.
 * @return Message A message with the row,col and direction of the ship.
 */
Message CleanPlayerV2::placeShip(int length) {
    char shipName[10];
    // Create ship names each time called: Ship0, Ship1, Ship2, ...
    snprintf(shipName, sizeof shipName, "Ship%d", numShipsPlaced);

    int row, col;
    Direction dir;

    if( length > boardSize ) return false;	// Ship too big for board

    while(true) {
	chooseValues(row, col, length, dir);
	if( positionOk(row, col, length, dir) ) {
	    numShipsPlaced++;
	    markShip(row, col, length, dir);
	    Message response( PLACE_SHIP, row, col, string(shipName), dir, length );

	    return response;
	}
    }
}

/*
 * Private internal function that chooses values for where to try placing a ship.
 */
void CleanPlayerV2::chooseValues( int& row, int& col, int& length, Direction& dir ) {
    dir = Direction( rand()%2 + 1 );
    if( dir==Horizontal ) {
        row = rand() % boardSize;
	col = rand() % (boardSize+1 - length);
    } else {	// vertical
	row = rand() % (boardSize+1 - length);
        col = rand() % boardSize;
    }
}

/*
 * Private internal function that determines whether a particular ship location is collision-free.
 */
bool CleanPlayerV2::positionOk( int row, int col, int length, Direction dir ) {
    if( dir==Horizontal ) {
	for(int c=col; c<col+length; c++) {
	    if(shipBoard[row][c] != WATER) return false;
	}
    } else {
	for(int r=row; r<row+length; r++) {
	    if(shipBoard[r][col] != WATER) return false;
	}
    }
    return true;
}

/*
 * Private internal function that marks a ship that is being placed.
 */
void CleanPlayerV2::markShip( int row, int col, int length, Direction dir ) {
    if( dir==Horizontal ) {
	for(int c=col; c<col+length; c++) {
	    shipBoard[row][c] = SHIP;
	}
    } else {
	for(int r=row; r<row+length; r++) {
	    shipBoard[r][col] = SHIP;
	}
    }
}

/*
 * Private internal function that follows up on hits.
 */
void CleanPlayerV2::getFollowUpShot( int& row, int& col ) {
    if ( search(row, col, 1, 0) ) {		// Down
	return;
    } else if ( search(row, col, 0, 1) ) {	// Right
	return;
    } else if ( search(row, col, 0, -1) ) { 	// Left
	return;
    } else {
	search(row, col, -1, 0);		// Up
	return;
    }
}

/*
 * Private internal function that looks along a particular path to see 
 * whether it can find something to shoot at.
 */
bool CleanPlayerV2::search( int& row, int& col, int rowDelta, int colDelta ) 
{
    int range=1;
    while( true ) {
	int r=row+rowDelta*range;
	int c=col+colDelta*range;

	if( ! isOnBoard(r,c) ) {
	    return false;
	} else if( board[r][c] == WATER ) {
	    row=r; col=c;
	    return true;
	} else if( board[r][c] == MISS || board[r][c] == KILL ) {
	    return false;
	} else { //	If it is a hit, just keep running through loop.
	    ;
	}
	range++;
    }

    return false;	// Guess we couldn't find anything.
}

/*
 * Private internal function that determines whether a location is on the board.
 */
bool CleanPlayerV2::isOnBoard( int row, int col ) {
    if( row>=0 && row<boardSize && col>=0 && col<boardSize )
        return true;
    else
        return false;
    // Or you could skip the if/else and just write
    //   return row>=0 && row<boardSize && col>=0 && col<boardSize;
}

/*
 * Private internal function that scans for the next place to shoot at.
 */
void CleanPlayerV2::scan( int& row, int& col ) {
    scanCol = scanCol + MIN_SHIP_SIZE;
    if( scanCol >= boardSize ) {
	scanCol = scanCol % boardSize;
	// if boardSize is multiple of MIN_SHIP_SIZE, could get caught going down columns. 
	// Adjust if needed.
	if( boardSize % MIN_SHIP_SIZE == 0 ) {	
	    if( scanCol + 1 == MIN_SHIP_SIZE ) {
		scanCol = 0;
	    } else {
		scanCol++;
	    }
	}
	scanRow++;
	if( scanRow >= boardSize ) {
	    scanRow = 0;
	}
    }
}

/**
 * @brief Tells the AI that a new round is beginning.
 * The AI show reinitialize any intra-round data structures.
 */
void CleanPlayerV2::newRound() {
    // Reinitialize any round-specific data structures here.

    this->scanRow = 0;
    this->scanCol = 0;
    this->numShipsPlaced = 0;
    this->shipMark = 'a';

    this->initializeBoard(this->board);
    this->initializeBoard(this->shipBoard);
}

/**
 * @brief Updates the AI with the results of its shots and where the opponent is shooting.
 * @param msg Message specifying what happened + row/col as appropriate.
 */
void CleanPlayerV2::update(Message msg) {
    switch(msg.getMessageType()) {
	case HIT:
	case KILL:
	case MISS:
	    board[msg.getRow()][msg.getCol()] = msg.getMessageType();
	    break;
	case OPPONENT_SHOT:
	    opponentShots[msg.getRow()][msg.getCol()]++;
	    cout << gotoRowCol(3, 30) << "CleanPl: opponent shot at "<< msg.getRow() << ", " << msg.getCol() << flush;
	    break;
    }
}
