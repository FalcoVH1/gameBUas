#pragma once
#include "math.h"
#include "template.h"
#include "surface.h"
#include <stdio.h>
#include <string.h>

namespace Tmpl8 {

class Surface;

enum class unitDirection {NORTH, EAST, SOUTH, WEST};
	
enum class unitType { MORTAR, RANGED, MELEE };

enum class unitMovementType { GROUND, NAVAL, AERIAL };

class coords {
public:
	int x = -1, y = -1;

	coords(int X, int Y) { x = X, y = Y; };

	bool equals( coords coordsToCompare) {
		if (x == coordsToCompare.x && y == coordsToCompare.y) {
			return true;
		} else return false;
	}

	coords() = default;

	coords multiply(int multiplier) {
		return coords{
			x *= multiplier,
			y *= multiplier
		};
	}

	coords divide(int divisor) {
		return coords{
			x /= divisor,
			y /= divisor
		};
	}

	coords add(coords addend) {
		return coords{
			x += addend.x,
			y += addend.y
		};
	}

	coords subtract(coords subtrahend) {
		return coords{
			x += subtrahend.x,
			y += subtrahend.y
		};
	}

	void set(int X, int Y) {
		x = X;
		y = Y;
	}
};

class Objective {
private:
	int x, y, health;
public:
	Objective(int X, int Y, int Health) {
		x = X;
		y = Y;
		health = Health;
	}

	void setHealth(int Health) {
		health = Health;
	}

	void takeDamage(int damageValue) {
		health -= damageValue;
		if (health <= 0) {
			x = -1;
			y = -1;
		}
	}

	int getHealth() {
		return health;
	}
	
	void kill() {
	
	}

	coords getCoords() {
		return {x, y};
	}

	int getX() {
		return x;
	}

	int getY() {
		return y;
	}

	void setX(int X) {
		x = X;
	}

	void setY(int Y) {
		y = Y;
	}
};

class Unit {
private:
	coords pastPlaces[15];
	int x = -1, 
		y = -1,
		health = -1,
		speed = -1,
		damageN = 0, //amount of damage unit can deal
		moveState = 0, //0 = hasn't moved, 1 = is moving, 2 = has moved
		attackState = 0, //0 = hasn't attacked, 1 = is selecting, 2 = is attacking
		moveCycle = 0; //indicates how many tiles a unit has moved
	bool isMoving = false;
	unitType attackType = unitType::MELEE;
	unitMovementType movementType = unitMovementType::GROUND;
	unitDirection uDir = unitDirection::NORTH;
public:
	Unit(int X, int Y, int Health, int Speed, int Damage, unitType Type, unitMovementType mvType) {
		x = X;
		y = Y;
		health = Health;
		speed = Speed;
		damageN = Damage;
		attackType = Type;
		movementType = mvType;
	}

	Unit() = default;

	void set(Unit u) {

		memcpy(pastPlaces, u.pastPlaces, sizeof(pastPlaces) / sizeof(pastPlaces[0]));

		x = u.x;
		y = u.y;
		health = u.health;
		speed = u.speed;
		damageN = u.damageN;
		moveState = u.moveState;
		attackState = u.attackState;
		moveCycle = u.moveCycle;
		isMoving = u.isMoving;
		attackType = u.attackType;
		movementType = u.movementType;
		uDir = u.uDir;
	}

	void setDir(unitDirection dir) {
		uDir = dir;
	}

	coords getPastPlaces(int index) {
		return pastPlaces[index];
	}

	int getMoveCycle() {
		return moveCycle;
	}

	void setMoveCycle(int nC) {
		moveCycle = nC;
	}

	coords getCoords() {
		return coords{ x, y };
	}

	bool getIsMoving() {
		return isMoving;
	}

	void setMoveState(int moved) {
		moveState = moved;
	}

	void setAttackState(int attacked) {
		attackState = attacked;
	}

	int getMoveState() {
		return moveState;
	}

	int getAttackState() {
		return attackState;
	}

	int getX() {
		return x;
	}

	int getY() {
		return y;
	}

	void setX(int X) {
		x = X;
	}

	void setY(int Y) {
		y = Y;
	}

	void setSpeed(int Speed) {
		speed = Speed;
	}

	void setHealth(int Health) {
		health = Health;
	}

	void takeDamage(int damageValue) {
		health -= damageValue;
		if (health <= 0) {
			x = -1;
			y = -1;
		}
	}

	int getDamage() {
		return damageN;
	}

	void setCoords(coords newCoords) {
		x = newCoords.x;
		y = newCoords.y;
	}

	int getHealth() {
		return health;
	}

	int getSpeed() {
		return speed;
	}

	void move(coords coordsToMoveTo) {

		int tempx = x - coordsToMoveTo.x;
		int tempy = y - coordsToMoveTo.y;

		if (tempx == 1 && tempy == 0) { uDir = unitDirection::WEST; }
		if (tempx == 0 && tempy == 1) { uDir = unitDirection::NORTH; }
		if (tempx == -1 && tempy == 0) { uDir = unitDirection::EAST; }
		if (tempx == 0 && tempy == -1) { uDir = unitDirection::SOUTH; }


		//code below stores array of last moves for units to go back without spending more tiles

		pastPlaces[moveCycle] = { x, y };

		int temp;
		if ((moveCycle - 1) <= 0) {
			temp = 0;
		}
		else {
			temp = moveCycle - 1;
		}


		if (coordsToMoveTo.x == pastPlaces[temp].x && coordsToMoveTo.y == pastPlaces[temp].y) {
			x = coordsToMoveTo.x;
			y = coordsToMoveTo.y;
			moveCycle--;
		}
		else {
			x = coordsToMoveTo.x;
			y = coordsToMoveTo.y;
			moveCycle++;
		}

		//fills coords of array with (-1, -1), a spot outside the map
		for (int t = moveCycle; t < speed; t++) {
			pastPlaces[t] = { -1, -1 };
		}
	}

	void clearPastPlaces() {
		memset(pastPlaces, 0, sizeof(pastPlaces));
	}

	unitType getUnitType() {
		return attackType;
	}

	int returnAttackArray(coords selected, int X, int Y) { //unitLocation is used exclusively because the AI requires checking a units attack array from any position on the map

		int diffX, diffY;
		if (selected.x - x != 0) {
			diffX = (selected.x - x) / abs(selected.x - x);
		}
		else diffX = 0;
		if (selected.y - y != 0) {
			diffY = (selected.y - y) / abs(selected.y - y);
		}
		else diffY = 0;

		int startVal, endVal;

		switch (attackType) {
		case unitType::MELEE: startVal = 1; endVal = 2;
			break;
		case unitType::RANGED: startVal = 1; endVal = 4;
			break;
		case unitType::MORTAR: startVal = 4; endVal = 5;
			break;
		default: startVal = 1; endVal = 2;
		}

		//finds coordinates --should probably replace 9 here but doesn't matter for now
		if ((x + diffX < 9 && x + diffX >= 0 && y + diffY < 9 && y + diffY >= 0)) {

			for (int t = startVal; t < endVal; t++) {
				if (x + diffX * t == X && y + diffY * t == Y) {
					return 1;
				}
			}
		}
		return 0;
	};

	unitMovementType getMvType() {
		return movementType;
	}
};


class gameState {
private:
	int friendlyUnitsAm = 0, enemyUnitsAm = 0;
	coords world = {0, 0};
	coords origin = { 0, 0 };

	
	int unitPlacement[9][9] = { 0 }; //map of all current units
	int worldField[9][9] = { 0 }; //terrain map

	bool canMove = true,
		areAttacking = false;
public: 

	//10 units max; units with 0 0 0 0 are automatically killed and don't appear on the field
	//so essentially; any amount of units up to 10
	Unit fUnits[10] = { Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND) };
	Unit eUnits[10] = { Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND), Unit(-1, -1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND) };

	Objective eObjtv = {0, 0, 0};
	Objective fObjtv = { 0, 0, 0 };

	//initializerlist was used so we can use coords and objective in the constructor class

	gameState(coords size, coords origin, Objective EOBJ, Objective FOBJ, int *arr, Unit FUnits[], Unit EUnits[], int fUnitsAm, int eUnitsAm) : 
		world(size.x, size.y),
		origin(origin.x, origin.y),
		eObjtv( EOBJ.getX(), EOBJ.getY(), EOBJ.getHealth() ), 
		fObjtv(FOBJ.getX(), FOBJ.getY(), FOBJ.getHealth() ) {

		friendlyUnitsAm = fUnitsAm;
		enemyUnitsAm = eUnitsAm;

		for (int x = 0; x < world.x; x++) {
			for (int y = 0; y < world.y; y++) {
				worldField[x][y] = *arr;
				arr++;
			}
		}
		
		for (int t = 0; t < friendlyUnitsAm; t++) {
			fUnits[t] = FUnits[t];
		}
		for (int t = 0; t < enemyUnitsAm; t++) {
			eUnits[t] = EUnits[t];
		}
	}

	gameState() = default;


	void set(gameState gs) {

		friendlyUnitsAm = gs.getFUnitsAm();
		enemyUnitsAm = gs.getEUnitsAm();

		world = gs.getWorld();
		origin = gs.getOrigin();

		for (int x = 0; x < world.x; x++) {
			for (int y = 0; y < world.y; y++) {
				unitPlacement[x][y] = gs.getUnitPlacement(x, y);
				worldField[x][y] = gs.getWorldField(x, y);
			}
		}

		canMove = gs.getCanMove();
		areAttacking = gs.getAreAttacking();

		for (int t = 0; t < friendlyUnitsAm; t++) {
			fUnits[t] = gs.fUnits[t];
		}
		for (int t = 0; t < enemyUnitsAm; t++) {
			eUnits[t] = gs.eUnits[t];
		}

		fObjtv = gs.fObjtv;
		eObjtv = gs.eObjtv;
	}


	//arrays

	void setUnitPlacement(int x, int y, int val) {
		unitPlacement[x][y] = val;
	}

	int getUnitPlacement(int x, int y) {
		return unitPlacement[x][y];
	}

	void setWorldField(int x, int y, int val) {
		worldField[x][y] = val;
	}

	int getWorldField(int x, int y) {
		return worldField[x][y];
	}


	//----------//


	void setFUnits(Unit FUnits[]) {

		//this could be done way cleaner; too bad!
		for (int t = 0; t < sizeof(fUnits) / sizeof(fUnits[0]); t++) {
			fUnits[t] = Unit(0, 0, 0, 0, 0, unitType::MELEE, unitMovementType::GROUND);
		}

		for (int t = 0; t < friendlyUnitsAm; t++) {
			fUnits[t] = FUnits[t];
		}
	}

	void setCanMove(bool canmove) {
		canMove = canmove;
	}

	bool getCanMove() {
		return canMove;
	}

	void setAreAttacking(bool areattacking) {
		areAttacking = areattacking;
	}

	bool getAreAttacking() {
		return areAttacking;
	}

	coords getWorld() {
		return world;
	}

	coords getOrigin() {
		return origin;
	}

	int getFUnitsAm() {
		return friendlyUnitsAm;
	}

	int getEUnitsAm() {
		return enemyUnitsAm;
	}
};

class Game
{
private:
	Surface* screen;
	int mousex, mousey;
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void MouseUp(int button) { /* implement if you want to detect mouse button presses */ };
	void MouseDown(int button);
	void MouseMove(int x, int y) { mousex = x, mousey = y; };
	void KeyUp(int key) { /* implement if you want to handle keys */ };
	void KeyDown(int key);

	vec2 getMousePos() {
		return vec2 {
			(float)mousex, (float)mousey
		};
	};

	coords getMousePosInt() {
		return coords{
			mousex, mousey
		};
	};

};



}; // namespace Tmpl8