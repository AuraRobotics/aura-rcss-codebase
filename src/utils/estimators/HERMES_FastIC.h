/*
 * HERMES_FastIC.h
 *
 *  Created on: Oct 10, 2013
 *      Author: amir
 */

#ifndef HERMES_FASTIC_H_
#define HERMES_FASTIC_H_

#include <rcsc/geom/vector_2d.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/player/player_object.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>

using namespace rcsc;
using namespace std;

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

#define MAX_CYCLE_ 150
#define MAX_BACK_CYCLE 5
#define SIDE_DASH_DIR_COUNT 3
#define MAX_GOALIE_BACK_CYCLE  15
#define MAX_SIDE_CYCLE 3
#define TYPESCOUNT 18
#define TURN_VELCOUNT 10
#define TURN_ANGLECOUNT 18

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

struct FastBall {
    Vector2D pos;
    Vector2D vel;
    float decay;
    int delay;

    FastBall(Vector2D _pos, Vector2D _vel, float _decay, float _delay = 0);

    FastBall() {
    }

};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

struct TurnModel {
    int cyclesToTurn[TURN_VELCOUNT][TURN_ANGLECOUNT];
    float maxVelocity;
    float maxAngleChange;

    int
    getCyclesToTurn(float velocity, float angleChange);

    TurnModel() {
        maxVelocity = 1.f;
        maxAngleChange = 180.f;
    }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

struct FastBody {
    const PlayerType *playerType;
    TurnModel Turn;
    Vector2D pos;
    Vector2D vel;
    float distance[MAX_CYCLE_];
    float backDistance[MAX_BACK_CYCLE];
    float sideDistance[SIDE_DASH_DIR_COUNT][MAX_SIDE_CYCLE];
    double effort;
    double stamina;
    AngleDeg body;

};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

struct FastestPlayer {
    bool fake;
    int unum;
    bool isTeammate;
    int reachCycle;
    Vector2D receivePoint;
    const AbstractPlayerObject *playerPtr;

    FastestPlayer(int _unum, bool _isTeammate, int _reachCycle, Vector2D _receivePoint,
                  const AbstractPlayerObject *_playerPtr = NULL, bool _fake =
    false);

    float reachProb;

    FastestPlayer() {
    }

    ~FastestPlayer() {
    }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

struct FastPlayer {
    bool fake;
    Vector2D pos;
    Vector2D vel;
    int delay;
    int extraDashCycles;
    float dashMulti;
    float controlMulti;
    bool calculated;
    const AbstractPlayerObject *playerPtr;
    bool hasDir;
    int exhaustionTime;
    SideID side;

    float tempProb;
    float mainProb;

    FastestPlayer *resultPlayer;

    FastPlayer(SideID _side, int _delay, float _controlBuff, float _dashMulti, const AbstractPlayerObject *_playerPtr,
               int _extraDashCycles = 0,
               bool _fake = false);

    FastPlayer() {
    }

    ~FastPlayer() {

        if (fake and playerPtr) {
            delete playerPtr;
        }
    }

    int
    getExhaustionTime();
};

class FastIC {
    static bool isSet;
    static FastBody *types[TYPESCOUNT];
    const PlayerAgent *agent;
    FastBall ball;
    vector<FastPlayer *> fastPlayers;
    vector<FastestPlayer *> fastestPlayers;
    const WorldModel &wm;
    int maxCycle, cyclesAfter, outCycle, maxOppCycle; // max cycle after opp first reach !
    bool outFlag;
    bool probMode;
    bool shootMode;
    float oppMinDist;
    float tmmMinDist;
    double maxTackleProb;
    float maxReachProb;

    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    void
    firstSetting();

    void
    bodySimulateDash(FastBody *body, double dashPower, double dir);

    void
    bodySimulate(FastBody *body);

    void
    ballSimulate(FastBall *ball);

    void clearFastPlayers() {
        for (int i = 0; i < fastPlayers.size(); i++) {
            delete fastPlayers[i];
        }
    }

    void clearFastestPlayers() {
        for (int i = 0; i < fastestPlayers.size(); i++) {
            delete fastestPlayers[i];
        }
    }

    float calculateReachProb(float deltaCycle, float deltaDist, float rand);

    void calcDashDist(int cycle, FastBall fb, FastPlayer *fp, float &distToBall, float &dashDist, float &directDashDist,
                      int &turnCycle);

    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
public:
    FastIC(const PlayerAgent *agent);

    virtual
    ~FastIC();

    static bool
    isPlayerValid(const AbstractPlayerObject *pl);

    static double
    getMaxSafelyDashPower(double power, double stamina);

    void
    setBall(Vector2D ballPos, Vector2D ballVel, int delay);

    void
    setBall(Vector2D ballPos, Vector2D ballVel, int delay, float decay);

    void
    addPlayer(const AbstractPlayerObject *player, float kickableMulti = 1.0, float dashMulti = 1.0, int delay = 0,
              int extraDashCycle = 0,
              bool _fake = false);

    void
    refresh();

    void
    reset();

    void
    calculate(bool debug = false);

    void
    setByWorldModel();

    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    //input functions
    void setMaxCycleAfterOppReach(unsigned max) {
        maxOppCycle = max;
    }

    void
    setMaxCycleAfterFirstFastestPlayer(unsigned max);

    void
    setMaxCycles(unsigned max);

    void setShootMode() {
        shootMode = true;
    }

    void setProbMode() {
        probMode = true;
    }

    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    //out functions
    bool isSelfFastestPlayer() const;

    bool isSelfFastestTeammate() const;

    Vector2D getFastestPlayerReachPoint() const;

    vector <FastestPlayer>
    getFastestTeammates() const;

    vector <FastestPlayer> getFastestOpponents() const;

    int getFastestTeammateReachCycle(bool withMe = true) const;

    Vector2D getFastestTeammateReachPoint(bool withMe = true) const;

    int getFastestOpponentReachCycle() const;

    Vector2D
    getFastestOpponentReachPoint() const;

    int
    getFastestPlayerReachCycle() const;

    float
    getFastestPlayerReachProb() const;

    int
    getSelfReachCycle() const;

    const AbstractPlayerObject *
    getFastestTeammate(bool withMe = true) const;

    const AbstractPlayerObject *
    getFastestOpponent() const;

    const AbstractPlayerObject *getFastestPlayer() const;

    const vector<FastestPlayer *>
    getFastestPlayers() const;

    float getOppMinDistToBall() {
        return max(0.f, oppMinDist);
    }

    float getTmmMinDistToBall() {
        return max(0.f, tmmMinDist);
    }

    float getOppMaxTackleProb() {
        return maxTackleProb;
    }

    int getOutCycle() {
        return outCycle;
    }

    float getMaxReachProb() {
        return maxReachProb;
    }
};

#endif /* HERMES_FASTIC_H_ */
