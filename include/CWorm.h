/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm class
// Created 28/6/02
// Jason Boettcher


#ifndef __CWORM_H__
#define __CWORM_H__

#include <cassert>
#include "LieroX.h" // for MAX_WORMS, _AI_DEBUG
#include "CProjectile.h"
#include "CNinjaRope.h"
#include "CWpnRest.h"
#include "Options.h" // for control_t
#include "Utils.h"
#include "Frame.h"
#include "DeprecatedGUI/CBar.h"
#include "CMap.h"


// TODO: remove this after we changed network
#include "CBytestream.h"


#define		MAX_WEAPONSLOTS		10

// Direction
#define		DIR_LEFT			0
#define		DIR_RIGHT			1

#define		WRM_OUT				-1
#define		WRM_UNLIM			-2

// AI Game Type
#define		GAM_RIFLES			0
#define		GAM_100LT			1
#define		GAM_MORTARS			2
#define		GAM_OTHER			3

// Cells
#define		CELL_CURRENT		0
#define		CELL_LEFT			1
#define		CELL_DOWN			2
#define		CELL_RIGHT			3
#define		CELL_UP				4
#define		CELL_LEFTDOWN		5
#define		CELL_RIGHTDOWN		6
#define		CELL_LEFTUP			7
#define		CELL_RIGHTUP		8


class CWpnRest;
class CGameScript;
class weapon_t;

// Weapon slot structure
struct wpnslot_t {
	const weapon_t	*Weapon;
	int			SlotNum;
	float		Charge;
	bool		Reloading;
	float		LastFire;
	bool		Enabled;
};

struct randweapons_t {
	int Weap1, Weap2, Weap3, Weap4, Weap5;
};

// the files have to be included yourself later
// they are used here; but their headers depends on this header
// TODO: remove the usage of these in this header
class CServerConnection;
class CBonus;


// Lobby worm details
#define		LBY_OPEN	0
#define		LBY_CLOSED	1
#define		LBY_USED	2

struct lobbyworm_t {
	int			iType;
    int         iTeam;
	bool		bHost;
	bool		bReady;
};



/*

    Artificial Intelligence


*/

// AI states
enum {
    AI_THINK,
    //AI_FINDTARGET,
    AI_MOVINGTOTARGET,
    AI_AIMING,
    AI_SHOOTING
};

// Target types
enum {
    AIT_NONE,
    AIT_WORM,
    AIT_BONUS,
    AIT_POSITION
};


class NEW_ai_node_t {
public:
	float fX, fY;
	NEW_ai_node_t *psPrev, *psNext;
};

NEW_ai_node_t* get_last_ai_node(NEW_ai_node_t* n);
void delete_ai_nodes(NEW_ai_node_t* start);
void delete_ai_nodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
float get_ai_nodes_length(NEW_ai_node_t* start);
// this do the same as the fct above except that it don't do the sqrt
float get_ai_nodes_length2(NEW_ai_node_t* start);



class CWorm {
public:
	// Constructor
	CWorm() {
		Clear();
	}

	~CWorm() {
		Shutdown();
	}

private:
	// disallow these!
	CWorm(const CWorm&) { assert(false); }
	CWorm& operator=(const CWorm&) { assert(false); }
	
private:
	// Attributes

	// General
	bool		bUsed;
	int			iID;
	int			iType;
	bool		bLocal;
	int			iTeam;
	std::string	sName;
	int			iRanking;
	int			iKillsInRow;
	int			iDeathsInRow;
	bool		bAlreadyKilled;
	bool		bSpectating;

	// Client info
	int			iClientID;
	int			iClientWormID;

	// Input
	CInput		cUp, cDown, cLeft, cRight,
				cShoot, cJump, cSelWeapon, cInpRope,
				cStrafe, cWeapons[5];
	bool		bUsesMouse;

	// last time we moved left or right
	float		lastMoveTime;

	// Simulation
	worm_state_t tState;
	CVec		vPos;
	CVec		vVelocity;
	CVec		vLastPos;
	CVec		vDrawPos;
	bool		bOnGround;
	float		fLastInputTime;

	CVec		vFollowPos;
	bool		bFollowOverride;

	// Score
	int			iKills;
	int			iDeaths;
	int			iSuicides;

	int			iTotalWins;
	int			iTotalLosses;
	int			iTotalKills;
	int			iTotalDeaths;
	int			iTotalSuicides;
	std::string	sAddressList;
	std::string sAliasList;

	// Game
	float		fLoadingTime;
	bool		bDrawMuzzle;
	int			iHealth;
	int			iLives;
	bool		bAlive;
	float		fTimeofDeath;
	int			iDirection;
	int			iMoveDirection;
	bool		bGotTarget;
	float		fAngle;
    float       fAngleSpeed;
    float		fMoveSpeedX;
	float		fFrame;
	CNinjaRope	cNinjaRope;
	profile_t	*tProfile;
	float		fRopeTime;

	bool		bHooked;
	CWorm		*pcHookWorm;

	bool		bRopeDown;
	bool		bRopeDownOnce;

	bool		bTagIT;
	float		fTagTime;
	float		fLastSparkle;

    int         iDirtCount;

	float		fLastBlood;

	CMap*		pcMap;


	// Owner client
	CServerConnection *cOwner;

	// Network
	float		fFrameTimes[NUM_FRAMES];
	lobbyworm_t tLobbyState;
		// server
	worm_state_t tLastState; // Used for checking if we need to send the packet
	float		fLastAngle;
	float		fLastUpdateWritten;
	CVec		vLastUpdatedPos; // last pos we have send to client
		// client
	float		fLastPosUpdate;  // Used for velocity calculations (client does not send velocity)
	CVec		vOldPosOfLastPaket;
	CVec		vPreOldPosOfLastPaket;
	float		fPreLastPosUpdate;
	CVec		vLastEstimatedVel;
	CVec		vPreLastEstimatedVel;
	byte		iLastCharge;
	byte		iLastCurWeapon;

	// Graphics
	CWormSkin	cSkin;
	SmartPointer<SDL_Surface> bmpGibs;
	DeprecatedGUI::CBar		cHealthBar;
	//CViewport	*pcViewport;


	// Arsenal
	bool		bWeaponsReady;
	bool		bIsPrepared;
	bool		bGameReady;
	CGameScript	*cGameScript;
    CWpnRest    *cWeaponRest;
	int			iNumWeaponSlots;
	int			iCurrentWeapon;
	wpnslot_t	tWeapons[MAX_WEAPONSLOTS];
	bool		bNoShooting;
    bool		bFlag;
	AFK_TYPE	iAFK;
	std::string	sAFKMessage;

    // Force the showing of the current weapon
    bool        bForceWeapon_Name;
    float       fForceWeapon_Time;




    /*
	    Artificial Intelligence
    */
    int         nAIState;
    int         nAITargetType;
    CWorm       *psAITarget;
    CBonus      *psBonusTarget;
    CVec        cPosTarget;
    int         nPathStart[2];
    int         nPathEnd[2];
    float       fLastCarve;
    CVec        cStuckPos;
    float       fStuckTime;
    bool        bStuck;
    float       fStuckPause;
    float       fLastThink;
    CVec        cNinjaShotPos;
	float		fLastFace;
	float		fSpawnTime;
	float		fBadAimTime;
    int			iAiGameType; // AI game type, we have predefined behaviour for mostly played settings
	int			iAiGame;
	int			iAiTeams;
	int			iAiTag;
	int			iAiVIP;
	int			iAiCTF;
	int			iAiTeamCTF;
	int			iAiDiffLevel;
	int			iRandomSpread;
	CVec		vLastShootTargetPos;

	float		fLastShoot;
	float		fLastJump;
	float		fLastWeaponChange;
	float		fLastCreated;
	float		fLastCompleting;
	float		fLastRandomChange;
	float		fLastGoBack;

	float		fCanShootTime;

	float		fRopeAttachedTime;
	float		fRopeHookFallingTime;

    // Path Finding
    int         nGridCols, nGridRows;
	float       fLastPathUpdate;
	bool		bPathFinished;
	float		fSearchStartTime;

	// its type is searchpath_base*; defined in CWorm_AI.cpp
	void*		pathSearcher;

	NEW_ai_node_t	*NEW_psPath;
	NEW_ai_node_t	*NEW_psCurrentNode;
	NEW_ai_node_t	*NEW_psLastNode;


public:
	// Methods


	//
	// General
	//
	void		Clear(void);
	void		Init(void);
	//void		CopyProfile(plyprofile_t *profile);
	void		Shutdown(void);


	//
	// Network
	//
	void		writeInfo(CBytestream *bs);
	void		readInfo(CBytestream *bs);
	static bool	skipInfo(CBytestream *bs)  { bs->SkipString(); bs->Skip(2); bs->SkipString(); return bs->Skip(3); }
	void		writeScore(CBytestream *bs);
	void		readScore(CBytestream *bs);
	static bool	skipScore(CBytestream *bs)  { return bs->Skip(3); }
	void		updateCheckVariables();
	bool		checkPacketNeeded();
	void		writePacket(CBytestream *bs, bool fromServer, CServerConnection* receiver);
	void		readPacket(CBytestream *bs, CWorm *worms);
	void		net_updatePos(const CVec& newpos);
	static bool	skipPacket(CBytestream *bs);
	void		readPacketState(CBytestream *bs, CWorm *worms);
	static bool	skipPacketState(CBytestream *bs)  {return skipPacket(bs); } // For skipping it's the same as skipPacket
	void		writeWeapons(CBytestream *bs);
	void		readWeapons(CBytestream *bs);
	static bool	skipWeapons(CBytestream *bs)  { return bs->Skip(5); } // 5 weapons
	void		writeStatUpdate(CBytestream *bs);
	void		updateStatCheckVariables();
	bool		checkStatePacketNeeded();
	void		readStatUpdate(CBytestream *bs);
	static bool	skipStatUpdate(CBytestream *bs) { return bs->Skip(2); } // Current weapon and charge
	int			GetMyPing(void);

	void		setupLobby(void);
	void		loadDetails(void);
	void		saveDetails(void);


	//
	// Weapon & Input
	//
	void		SetupInputs(const controls_t& Inputs);
	void		InitWeaponSelection(void);
	void		GetRandomWeapons(void);
	void		SelectWeapons(SDL_Surface * bmpDest, CViewport *v);
	void		InitInputSystem();
	void		StopInputSystem();


	//
	// Graphics
	//
	bool		ChangeGraphics(int gametype);
	void		FreeGraphics(void);
	SmartPointer<SDL_Surface> ChangeGraphics(const std::string& filename, int team);
	void		Draw(SDL_Surface * bmpDest, CViewport *v);
    void        DrawShadow(SDL_Surface * bmpDest, CViewport *v);
	void		UpdateDrawPos();

	//
	// Game
	//
	bool		isPrepared() { return bIsPrepared; }
	void		Prepare(CMap *pcMap);
	void		Unprepare(); // after a game
	void		StartGame();
	void		Spawn(CVec position);
	void		Respawn(CVec position);
	bool		Injure(int damage);
	bool		Kill(void);
	bool		CheckBonusCollision(CBonus *b);
	bool		GiveBonus(CBonus *b);



	//
	// Simulation
	//
	void		getInput(void);
    void        clearInput(void);
	void		getGamepadInput(void);
    bool		CheckOnGround();


	//
	// Misc.
	//
	bool		CanType(void);



    //
    // AI
    //
    bool        AI_Initialize();
    void        AI_Shutdown(void);

	// TODO: what is the sense of all these parameters? (expect gametype)
	void		AI_GetInput(int gametype, int teamgame, int taggame, int VIPgame, int flaggame, int teamflaggame);
	void		AI_Respawn();
	// TODO: what is the sense of all these parameters?
    void        AI_Think(int gametype, int teamgame, int taggame);
    bool        AI_FindHealth();
    bool        AI_SetAim(CVec cPos);
    CVec        AI_GetTargetPos(void);

    void        AI_InitMoveToTarget();
    void        AI_SimpleMove(bool bHaveTarget=true);
//    void        AI_PreciseMove();

    void		AI_splitUpNodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
    void		AI_storeNodes(NEW_ai_node_t* start, NEW_ai_node_t* end);

    int         AI_FindClearingWeapon();
    bool        AI_Shoot();
    int         AI_GetBestWeapon(int nGameType, float fDistance, bool bDirect, float fTraceDist);
    void        AI_ReloadWeapons();
    int         cycleWeapons();
	void		AI_SetGameType(int type)  { iAiGameType = type; }
	int			AI_GetGameType()  { return iAiGameType; }

    CWorm       *findTarget(int gametype, int teamgame, int taggame);
    int         traceLine(CVec target, float *fDist, int *nType, int divs = 5);
	int			traceLine(CVec target, CVec start, int *nType, int divs = 5, uchar checkflag = PX_EMPTY);
	int			traceWeaponLine(CVec target, float *fDist, int *nType);
	bool		weaponCanHit(int gravity,float speed, CVec cTrgPos);
	bool		IsEmpty(int Cell);
    //void        moveToTarget(CWorm *pcTarget);

	CVec		AI_GetBestRopeSpot(CVec trg);
	CVec		AI_FindClosestFreeCell(CVec vPoint);
	bool		AI_CheckFreeCells(int Num);
	bool		AI_IsInAir(CVec pos, int area_a=3);
	CVec		AI_FindClosestFreeSpotDir(CVec vPoint, CVec vDirection, int Direction);
	CVec		AI_FindBestFreeSpot(CVec vPoint, CVec vStart, CVec vDirection, CVec vTarget, CVec* vEndPoint);
	int			AI_CreatePath(bool force_break = false);
	void		AI_MoveToTarget();
	CVec		AI_GetNearestRopeSpot(CVec trg);
	void		AI_Carve();
	bool		AI_Jump();
	CVec		AI_FindShootingSpot();
	int			AI_GetRockBetween(CVec pos,CVec trg, CMap *pcMap);
#ifdef _AI_DEBUG
	void		AI_DrawPath();
#endif



    //int         getBestWeapon(int nGameType, float fDistance, CVec cTarget);







	//
	// Variables
	//
	bool		isUsed(void)				{ return bUsed; }
	void		setUsed(bool _u);

	CMap*		getMap()					{ return pcMap; }
	void		setMap(CMap *map)			{ pcMap = map; }
	CNinjaRope*	getNinjaRope()				{ return &cNinjaRope; }
	CInput*		getStrafeInput()			{ return &cStrafe; }
	CInput*		getShootInput()				{ return &cShoot; }

	std::string getName(void)			{ return sName; }
	void		setName(const std::string& val) { sName = val; }
	Uint32		getGameColour(void)			{
		switch(tGameInfo.iGameMode) {
		case GMT_TEAMDEATH:
		case GMT_VIP:
		case GMT_TEAMCTF:
			return tLX->clTeamColors[iTeam];
		default:
			return cSkin.getDefaultColor();
		}
	}
	void		setColour(Uint32 c)			{ cSkin.Colorize(c); }
	void		setColour(Uint8 r, Uint8 g, Uint8 b) {cSkin.Colorize(MakeColour(r,g,b)); }

	void		setLocal(bool _l)			{ bLocal = _l; }
	bool		getLocal(void)				{ return bLocal; }

	int			getHealth(void)				{ return iHealth; }
	void		setHealth(int _h)			{ iHealth = CLAMP(_h, 0, 100); }
	int			getLives(void)				{ return iLives; }
	int			getKills(void)				{ return iKills; }
	void		setLives(int l)				{ iLives = l; }

	void		AddKill(void)				{ iKills++; }
    void        setKills(int k)             { iKills = k; }

	void		setID(int i)				{ iID = i; }
	int			getID(void)					{ return iID; }

	int			getType(void)				{ return iType; }
    void        setType(int t)              { iType = t; }

	bool		getAlive(void)				{ return bAlive; }
	void		setAlive(bool _a)			{ bAlive = _a; }

	float		getTimeofDeath(void)		{ return fTimeofDeath; }

	void		setHooked(bool h, CWorm *w)	{ bHooked=h; pcHookWorm=w; }
	void		setClient(CServerConnection *cl)		{ cOwner = cl; }
    CServerConnection     *getClient(void)            { return cOwner; }

	CInput		*getShoot(void)				{ return &cShoot; }

	CVec		getFollowPos(void)			{ return (bFollowOverride?vFollowPos:vPos); }
	void		resetFollow(void)			{ bFollowOverride = false; }
	void		doFollow(int x, int y)		{ bFollowOverride = true; vFollowPos.x = (float)x; vFollowPos.y = (float)y; }

	CVec		getPos(void)				{ return vPos; }
	void		setPos(CVec v)				{ vPos = v; }

	CVec&		pos()						{ return vPos; }
	bool		isOnGround()				{ return bOnGround; }
	void		setOnGround(bool g)			{ bOnGround = g; }

	CVec		*getVelocity(void)			{ return &vVelocity; }
	CVec&		velocity()					{ return vVelocity; }

	worm_state_t *getWormState(void)		{ return &tState; }

	float		getAngle(void)				{ return fAngle; }
	void		setAngle(float a)			{ fAngle = a; }
	int			getDirection(void)			{ return iDirection; }
	void		setDirection(int d)			{ iDirection = d; }
	int			getMoveDirection()		{ return iMoveDirection; }

	void		setLoadingTime(float l)		{ fLoadingTime = l; }
	float		getLoadingTime()			{ return fLoadingTime; }

	CGameScript* getGameScript()			{ return cGameScript; }
	void		setGameScript(CGameScript *gs)	{ cGameScript = gs; }
    void        setWpnRest(CWpnRest *wr)    { cWeaponRest = wr; }

	void		setDrawMuzzle(bool _d)		{ bDrawMuzzle = _d; }

	bool		getWeaponsReady(void)		{ return bWeaponsReady; }
	void		setWeaponsReady(bool _w)	{ bWeaponsReady = _w; }
	wpnslot_t	*getCurWeapon(void)			{ return &tWeapons[MIN(4, iCurrentWeapon)]; }
	int			getCurrentWeapon(void)		{ return MIN(4, iCurrentWeapon); }
	void		setCurrentWeapon(int _w)	{ iCurrentWeapon = MIN(4,_w); }
	wpnslot_t	*getWeapon(int id)			{ return &tWeapons[id]; }

	void		setGameReady(bool _g)		{ bGameReady = _g; }
	bool		getGameReady(void)			{ return bGameReady; }

	void		setProfile(profile_t *p)	{ tProfile = p; }
	profile_t	*getProfile()				{ return tProfile; }

	void		setTeam(int _t)				{ iTeam = _t; }
	int			getTeam(void)				{ return iTeam; }

	SmartPointer<SDL_Surface> getGibimg(void)			{ return bmpGibs; }
	SmartPointer<SDL_Surface> getPicimg(void)			{ return cSkin.getPreview(); }

	lobbyworm_t	*getLobby(void)				{ return &tLobbyState; }

	bool		getTagIT(void)				{ return bTagIT; }
	void		setTagIT(bool _t)			{ bTagIT = _t; }

	float		getLastSparkle()			{ return fLastSparkle; }
	void		setLastSparkle(float s)		{ fLastSparkle = s; }
	float		getLastBlood()				{ return fLastBlood; }
	void		setLastBlood(float b)		{ fLastBlood = b; }

    void        incrementDirtCount(int d)   { iDirtCount += d; }
    int         getDirtCount(void)          { return iDirtCount; }

	void		setTarget(bool _t)			{ bGotTarget = _t; }

	float		getTagTime(void)			{ return fTagTime; }
	void		setTagTime(float _t)		{ fTagTime = _t; }
	void		incrementTagTime(float dt)	{ fTagTime+=dt; }

	CWormSkin&	getSkin(void)				{ return cSkin; }
	void		setSkin(const CWormSkin& skin)	{ cSkin = skin; }
	void		setSkin(const std::string& skin)	{ cSkin.Change(skin); }

	void		setKillsInRow(int _k)		{ iKillsInRow = 0; }
	int			getKillsInRow(void)			{ return iKillsInRow; }
	void		addKillInRow(void)			{ iKillsInRow++; }

	void		setDeathsInRow(int _k)		{ iDeathsInRow = 0; }
	int			getDeathsInRow(void)		{ return iDeathsInRow; }
	void		addDeathInRow(void)			{ iDeathsInRow++; }

	bool		getAlreadyKilled()			{ return bAlreadyKilled; }
	void		setAlreadyKilled(bool _k)	{ bAlreadyKilled = _k; }

	bool		isShooting()				{ return tState.bShoot; }
	bool		isWeaponReloading()			{ return getCurWeapon()->Reloading; }

	bool		getVIP(void)				{ return bNoShooting; }
	void		setVIP(bool _s)				{ bNoShooting = _s; }

	bool		isSpectating()				{ return bSpectating; }
	void		setSpectating(bool _s)		{ bSpectating = _s; }

	// TODO: the sense of this isn't clear; so make it clear
	bool		getFlag(void)				{ return bFlag; }
	void		setFlag(bool _f)			{ bFlag = _f; bNoShooting = _f; }

	AFK_TYPE	getAFK(void)				{ return iAFK; }
	const std::string & getAFKMessage()		{ return sAFKMessage; }
	void		setAFK(AFK_TYPE _f, const std::string & msg);

	void	addTotalWins(int _w = 1)		{ iTotalWins += _w; }
	int		getTotalWins(void)				{ return iTotalWins; }
	void	addTotalLosses(int _l = 1)		{ iTotalLosses += _l; }
	int		getTotalLosses(void)			{ return iTotalLosses; }
	void	addTotalKills(int _k = 1)		{ iTotalKills += _k; }
	int		getTotalKills(void)				{ return iTotalKills; }
	void	addTotalDeaths(int _d = 1)		{ iTotalDeaths += _d; }
	int		getTotalDeaths(void)			{ return iTotalDeaths; }
	void	addTotalSuicides(int _d = 1)	{ iTotalSuicides += _d; }
	int		getTotalSuicides(void)			{ return iTotalSuicides; }

	std::string		getAddresses(void)			{ return sAddressList; }
	void			setAddresses(std::string _s){ sAddressList = _s; }
	std::string		getAliases(void)			{ return sAliasList; }
	void			setAliases(std::string _s)	{ sAliasList = _s; }

	float&		frame()						{ return fFrame; }

	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateWorms()
	// there which simulates all worms together
	float	fLastSimulationTime;
	
	CInput &	getInputUp()					{ return cUp; };
	CInput &	getInputDown()					{ return cDown; };
	CInput &	getInputLeft()					{ return cLeft; };
	CInput &	getInputRight()					{ return cRight; };
	CInput &	getInputShoot()					{ return cShoot; };
	CInput &	getInputJump()					{ return cJump; };
	CInput &	getInputWeapon()				{ return cSelWeapon; };
	CInput &	getInputRope()					{ return cInpRope; };
	CInput &	getInputStrafe()				{ return cStrafe; };
};


int traceWormLine(CVec target, CVec start, CMap *pcMap, CVec* collision = NULL);


#endif  //  __CWORM_H__
