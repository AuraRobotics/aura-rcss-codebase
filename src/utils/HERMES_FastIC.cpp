/*
 * HERMES_FastIC.cpp
 *
 *  Created on: Oct 10, 2013
 *      Author: amir
 */

#include "HERMES_FastIC.h"
#include <fstream>
using namespace std;

//#define FastICLOG

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

#define EXTRA_DASH_RATE 0.7
#define EXHAUSTION_RATE 0.1

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

bool FastIC::isSet = false;
FastBody * FastIC::types[TYPESCOUNT];

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

FastBall::FastBall(Vector2D _pos, Vector2D _vel, float _decay, float _delay)
		: pos(_pos), vel(_vel), decay(_decay), delay(_delay)
{
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

FastestPlayer::FastestPlayer(int _unum, bool _isTeammate, int _reachCycle, Vector2D _receivePoint, const AbstractPlayerObject * _playerPtr,
		bool _fake)
		: unum(_unum), isTeammate(_isTeammate), reachCycle(_reachCycle), receivePoint(_receivePoint), playerPtr(_playerPtr), fake(_fake), reachProb(
				0.f)
{
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

FastPlayer::FastPlayer(SideID _side, int _delay, float _controlBuff, float _dashMulti, const AbstractPlayerObject * _playerPtr, int _extraDashCycles,
		bool _fake)
		: pos(0.f, 0.f), vel(0.f, 0.f), delay(_delay), extraDashCycles(_extraDashCycles), dashMulti(_dashMulti), controlMulti(_controlBuff), calculated(
				false), playerPtr(_playerPtr), hasDir(false), fake(_fake), side(_side), tempProb(0.f), mainProb(0.f), resultPlayer(NULL)
{
	exhaustionTime = getExhaustionTime();
	if (_playerPtr)
	{
		pos = _playerPtr->pos();
		vel = _playerPtr->vel();
		if (playerPtr->bodyCount() == 0 || playerPtr->isSelf())
			hasDir = true;
	}
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


int FastPlayer::getExhaustionTime()
{
    int exhaustionTime[11] = {150,150,150,150,150,150,150,150,150,150,150};
	if (playerPtr and playerPtr->side() == side and playerPtr->unum() != Unum_Unknown)
	{
		return exhaustionTime[playerPtr->unum() - 1];
	}
	return 150;

}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

FastIC::FastIC(const PlayerAgent * agent)
		: agent(agent), wm(agent->world()), maxCycle(100), cyclesAfter(100), outCycle(0), outFlag(false), shootMode(false), oppMinDist(1000.f), maxTackleProb(
				0.0), probMode(false), maxOppCycle(100), tmmMinDist(1000.f), maxReachProb(0.0)
{
	clearFastPlayers();
	fastPlayers.clear();
	clearFastestPlayers();
	fastestPlayers.clear();
	if (!isSet)
	{
		firstSetting();
		isSet = true;
	}
}

FastIC::~FastIC()
{
	clearFastPlayers();
	fastPlayers.clear();
	clearFastestPlayers();
	fastestPlayers.clear();
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void FastIC::ballSimulate(FastBall * ball)
{
	ball->pos = ball->pos + ball->vel;
	ball->vel *= ball->decay;

}
void FastIC::bodySimulate(FastBody * body)
{
	body->pos += body->vel;
	body->vel *= body->playerType->playerDecay();
}
void FastIC::bodySimulateDash(FastBody * body, double dashPower, double dir)
{
	Vector2D dashAcclerate;
	double dashDirRate = ((fabs(dir - 0.0) < 0.001) or (fabs(dir - 180.0) < 0.001)) ? 1.0 : ServerParam::i().dashDirRate(dir);
	dashAcclerate = Vector2D::polar2vector(fabs(dashPower) * body->playerType->dashPowerRate() * body->effort * dashDirRate, AngleDeg(0.0));
	body->vel += dashAcclerate;
	if (body->vel.length() > body->playerType->playerSpeedMax())
		body->vel = Vector2D::polar2vector(body->playerType->playerSpeedMax(), body->vel.dir());
	body->stamina += body->playerType->staminaIncMax();
	if (dashPower >= 0.0)
		body->stamina -= dashPower;
	else
		body->stamina += dashPower * 2.0;
#ifdef FastICLOG
	dlog.addText(Logger::TEAM, __FILE__" SIMULATING BALL");
	dlog.addText(Logger::TEAM, __FILE__" Player speed max %f", body->playerType->playerSpeedMax());
	dlog.addText(Logger::TEAM, __FILE__" DashPower: %f DashRate: %f effort %f", dashPower,
			body->playerType->dashPowerRate(), body->effort);
	dlog.addText(Logger::TEAM, __FILE__" Dash accle: %f", dashAcclerate.length());
	dlog.addText(Logger::TEAM, __FILE__" DashVel: %f", body->vel.length());
	dlog.addText(Logger::TEAM, __FILE__" Body Stamina %f", body->stamina);
#endif
}
int TurnModel::getCyclesToTurn(float velocity, float angleChange)
{
	int velocityBlock = min((int) (velocity / (maxVelocity / (float) TURN_VELCOUNT)),
	TURN_VELCOUNT - 1);
	int angleBlock = min((int) (angleChange / (maxAngleChange / (float) TURN_ANGLECOUNT)),
	TURN_ANGLECOUNT - 1);
	return cyclesToTurn[velocityBlock][angleBlock];
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

double FastIC::getMaxSafelyDashPower(double dash, double stamina)
{

	double required_stamina = (dash > 0.0 ? dash : dash * -2.0);
	const double safety_stamina = stamina - (ServerParam::i().recoverDecThr() * ServerParam::i().staminaMax()) - 3.0;
	double available_stamina = std::max(0.0, safety_stamina);
	double result_power = std::min(required_stamina, available_stamina);

	if (dash < 0.0)
		result_power *= -0.5;

	if (std::fabs(result_power) > std::fabs(dash))
		return dash;

	return result_power;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void FastIC::firstSetting()
{
	for (int i = 0; i < TYPESCOUNT; i++)
	{
		AbstractPlayerObject * player = new PlayerObject;
		player->setPlayerType(i);
		types[i] = new FastBody();
		types[i]->playerType = player->playerTypePtr();
		types[i]->effort = player->playerTypePtr()->effortMax();
		types[i]->stamina = ServerParam::i().staminaMax();
		types[i]->pos = Vector2D(0, 0);
		types[i]->body = AngleDeg(0);
		types[i]->vel = Vector2D(0, 0);
		for (int cycle = 0; cycle < MAX_CYCLE_; cycle++)
		{
			types[i]->distance[cycle] = types[i]->pos.x;
#ifdef FastICLOG
			dlog.addText(Logger::TEAM, __FILE__" distance num %i = %f", cycle,
					types[i]->distance[cycle]);
#endif
			float dashPower = getMaxSafelyDashPower(100, types[i]->stamina);
			bodySimulateDash(types[i], dashPower, 0.0);
			bodySimulate(types[i]);
		}

		///////////////////// BACK DASH
		types[i]->stamina = ServerParam::i().staminaMax();
		types[i]->pos = Vector2D(0, 0);
		types[i]->body = AngleDeg(0);
		types[i]->vel = Vector2D(0, 0);

		for (int cycle = 0; cycle < MAX_GOALIE_BACK_CYCLE; cycle++)
		{
			types[i]->backDistance[cycle] = types[i]->pos.x;
			;
#ifdef FastICLOG
			dlog.addText(Logger::TEAM, __FILE__" distance num %i = %f", cycle,
					types[i]->distance[cycle]);
#endif
			float dashPower = getMaxSafelyDashPower(100, types[i]->stamina);
			bodySimulateDash(types[i], dashPower * -1.0, 180.0);
			bodySimulate(types[i]);
		}
		///////////////////// SIDE DASH
		for (int dir = 0; dir < SIDE_DASH_DIR_COUNT; dir++)
		{
			types[i]->stamina = ServerParam::i().staminaMax();
			types[i]->pos = Vector2D(0, 0);
			types[i]->body = AngleDeg(0);
			types[i]->vel = Vector2D(0, 0);
			for (int cycle = 0; cycle < MAX_SIDE_CYCLE; cycle++)
			{
				types[i]->sideDistance[dir][cycle] = types[i]->pos.x;
#ifdef FastICLOG
				dlog.addText(Logger::TEAM, __FILE__" distance num %i = %f", cycle,
						types[i]->distance[cycle]);
#endif
				float dashPower = getMaxSafelyDashPower(100, types[i]->stamina);
				double dashDir = (180.0 / (SIDE_DASH_DIR_COUNT + 1)) * (dir + 1);
				bodySimulateDash(types[i], dashPower, dashDir);
				bodySimulate(types[i]);
			}
		}

		float rateStep = types[i]->playerType->playerSpeedMax() / (float) TURN_VELCOUNT;
		types[i]->Turn.maxVelocity = types[i]->playerType->playerSpeedMax();
		for (unsigned int rateCount = 0; rateCount < TURN_VELCOUNT; rateCount++)
		{
			float thisRate = rateCount * rateStep;
			float angleStep = 180.f / (float) TURN_ANGLECOUNT;
			for (unsigned int angleCount = 0; angleCount < TURN_ANGLECOUNT; angleCount++)
			{
				float threshold = 12.5f;
				float thisAngle = angleCount * angleStep;
				int turnCycles = 0;
				float calculatingRate = thisRate;
				while (true)
				{
					float turnAngle = thisAngle;
					if (/*std::fabs*/(turnAngle) < threshold)
						break;
					/*					turnAngle = turnAngle
					 * (1
					 + types[i]->playerType->inertiaMoment()
					 * calculatingRate);
					 if (std::fabs(turnAngle) > ServerParam::i().maxMoment())
					 turnAngle = AngleDeg::normalize_angle(
					 (turnAngle / std::fabs(turnAngle))
					 * ServerParam::i().maxMoment());
					 turnAngle = turnAngle
					 / (1
					 + types[i]->playerType->inertiaMoment()
					 * calculatingRate);*/

					turnAngle = ServerParam::i().maxMoment() / (1 + types[i]->playerType->inertiaMoment() * calculatingRate);
					calculatingRate *= types[i]->playerType->playerDecay();
					thisAngle -= turnAngle;
					turnCycles++;
				}
				types[i]->Turn.cyclesToTurn[rateCount][angleCount] = turnCycles;
			}
		}
	}
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

float erf(float x)
{
	float sign = 1;

	static bool set = false;
	static vector<float> erfs;
	if (!set)
	{
		float a1 = 0.254829592;
		float a2 = -0.284496736;
		float a3 = 1.421413741;
		float a4 = -1.453152027;
		float a5 = 1.061405429;
		float p = 0.3275911;

		erfs.resize(101);
		for (int i = 0; i <= 100; i++)
		{
			float y = (float) i * 0.05;
			float t = 1.0 / (1.0 + p * y);
			erfs[i] = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x); // exp?
		}
	}
	if (x < 0)
	{
		sign = -1;
		x *= -1;
	}

	if (x > 5)
		return sign;

	return erfs[((int) (x / 100))] * sign;
}

float FastIC::calculateReachProb(float deltaCycle, float deltaDist, float rand)
{
	float cycleRand = rand * deltaCycle;

	/*
	 3 * sigma = cycleRand
	 sigma = cycleRand / 3
	 sqrt(2) * sigma = cycleRand * sqrt(2)/3
	 sqrt(2) * sigma = 0.47140452079103168293 * cycleRand
	 */

	if (deltaCycle <= 3)
		return bound(0.00001, (double) (deltaDist + cycleRand) / (2 * cycleRand), 0.99999);
	else
		return bound(0.00001, 0.5 + 0.5 * erf(deltaDist / (0.4714 * cycleRand)), 0.99999);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void FastIC::calcDashDist(int cycle, FastBall fb, FastPlayer * fp, float &outDistToBall, float &outDashDist, float &outDirectDashDist,
		int &outTurnCycle)
{
	int playerType = fp->playerPtr->type();
	if (!(fp->playerPtr->type() >= 0 && fp->playerPtr->type() <= 17))
		playerType = 0;

	int turnCycle = 0;
	int backTurnCycle = 0;
	int sideDashDir = 0;

	float distToBall = fp->pos.dist(fb.pos);

	if (fp->hasDir && fp->playerPtr->velCount() < 10)
	{
		Vector2D itPosToBall;
		itPosToBall = fp->pos - fb.pos;
		float targetDir = (fb.pos - fp->pos).dir().degree();
		float dirChange;
		float backDirChange;
		dirChange = fabs(AngleDeg::normalize_angle(fp->playerPtr->body().degree() - targetDir));

		backDirChange = 180.0 - dirChange;

		float minDir = 0.0;
		if (fp->controlMulti * types[playerType]->playerType->kickableArea() <= distToBall)
			minDir = AngleDeg::asin_deg(fp->controlMulti * types[playerType]->playerType->kickableArea() / distToBall);
		if (dirChange <= minDir)
			dirChange = 0.0;
		if (backDirChange <= minDir)
			backDirChange = 0.0;
		turnCycle = types[playerType]->Turn.getCyclesToTurn(Vector2D(fp->vel).length(), dirChange);
		backTurnCycle = types[playerType]->Turn.getCyclesToTurn(Vector2D(fp->vel).length(), backDirChange);

		if (dirChange < 67.5)
			sideDashDir = 0;
		else if (dirChange < 112.5)
			sideDashDir = 1;
		else
			sideDashDir = 2;

	}

	/////////////////////
	float directDashDist = types[playerType]->distance[max(0, min(cycle - fp->delay - turnCycle, fp->exhaustionTime))]
			+ (max(0, cycle - fp->exhaustionTime) * EXHAUSTION_RATE);
	directDashDist += EXTRA_DASH_RATE * fp->extraDashCycles;
	directDashDist *= fp->dashMulti;
	/////////////////////
	float backDashDist = 0.0;

	if (fp->playerPtr->side() != wm.ourSide() or Rect2D(Vector2D(-20.0, 36.0), Vector2D(52.5, 20)).contains(fp->playerPtr->pos()))
	{

		int backCycle = max(0, min(cycle - fp->delay - backTurnCycle, (int) (fp->exhaustionTime / 2.0)));
		int maxCycle = fp->playerPtr->goalie() ? MAX_GOALIE_BACK_CYCLE : MAX_BACK_CYCLE;
		backCycle = min(backCycle, maxCycle - 1);

		backDashDist = types[playerType]->backDistance[backCycle] + (max(0, cycle - fp->exhaustionTime) * EXHAUSTION_RATE);
		backDashDist += EXTRA_DASH_RATE * fp->extraDashCycles;
		backDashDist *= fp->dashMulti;
	}
	/////////////////////

	float sideDashDist = 0.0;

	int sideCycle = max(0, min(cycle - fp->delay, fp->exhaustionTime));
	sideCycle = min(sideCycle, MAX_SIDE_CYCLE - 1);

	sideDashDist = types[playerType]->sideDistance[sideDashDir][sideCycle] + (max(0, cycle - fp->exhaustionTime) * EXHAUSTION_RATE);
	sideDashDist += EXTRA_DASH_RATE * fp->extraDashCycles;
	sideDashDist *= fp->dashMulti;

	float dashDist = fp->playerPtr->side() == wm.ourSide() ? max(directDashDist, backDashDist) : max(sideDashDist, max(directDashDist, backDashDist));
	/////////////////////

	outTurnCycle = turnCycle;
	outDirectDashDist = directDashDist;
	outDistToBall = distToBall;
	outDashDist = dashDist;

}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void FastIC::setByWorldModel()
{
	clearFastPlayers();
	fastPlayers.clear();
	clearFastestPlayers();
	fastestPlayers.clear();
	for (int i = 0; i < wm.ourPlayers().size(); i++)
	{
		if (isPlayerValid(wm.ourPlayers()[i]))
		{
			addPlayer(wm.ourPlayers()[i]);

		}
	}
	for (int i = 0; i < wm.theirPlayers().size(); i++)
	{
		if (isPlayerValid(wm.theirPlayers()[i]))
		{
			addPlayer(wm.theirPlayers()[i]);

		}
	}
	setBall(wm.ball().pos(), wm.ball().vel(), 0, ServerParam::i().ballDecay());
	setMaxCycleAfterFirstFastestPlayer(20);
	setMaxCycleAfterOppReach(20);
	setMaxCycles(20);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void FastIC::addPlayer(const AbstractPlayerObject * _player, float _kickableMulti, float _dashMulti, int _delay, int _extraDashCycle, bool _fake)
{
	if (_player->side() == wm.ourSide())
	{
		_kickableMulti = 1.0 - (0.15 / _player->playerTypePtr()->kickableArea());
	}

	if (_player->isTackling() and dynamic_cast<const PlayerObject *>(_player))

	{
		const PlayerObject * pl = dynamic_cast<const PlayerObject *>(_player);
		_delay += std::max(0, ServerParam::i().tackleCycles() - pl->tackleCount() - 2);
		int dif = _extraDashCycle - _delay;
		if (dif == 0)
		{
			_delay = 0;
			_extraDashCycle = 0;
		}
		if (dif > 0)
		{
			_delay = 0;
			_extraDashCycle = dif;
		}
		if (dif < 0)
		{
			_delay = abs(dif);
			_extraDashCycle = 0;
		}

	}

	fastPlayers.push_back(new FastPlayer(wm.ourSide(), _delay, _kickableMulti, _dashMulti, _player, _extraDashCycle, _fake));

}
void FastIC::setBall(Vector2D ballPos, Vector2D ballVel, int delay)
{
    ball = FastBall(ballPos, ballVel, ServerParam::i().ballDecay(), delay);
}
void FastIC::setBall(Vector2D ballPos, Vector2D ballVel, int delay, float decay)
{
    ball = FastBall(ballPos, ballVel, decay, delay);
}
bool FastIC::isPlayerValid(const AbstractPlayerObject * pl)
{
	return (pl && ((pl->playerTypePtr() && pl->posCount() < 30) || pl->isSelf()));
}
void FastIC::refresh()
{
	oppMinDist = 1000.f;
	tmmMinDist = 1000.f;
	maxTackleProb = 0.0;
	maxReachProb = 0.0;
	outFlag = false;
	outCycle = 1000;
	for (int i = 0; i < fastPlayers.size(); i++)
	{
		fastPlayers[i]->resultPlayer = NULL;
		fastPlayers[i]->tempProb = 0.0;
		fastPlayers[i]->mainProb = 0.0;
		if (fastPlayers[i]->playerPtr)
		{
			fastPlayers[i]->pos = fastPlayers[i]->playerPtr->pos();
			fastPlayers[i]->vel = fastPlayers[i]->playerPtr->vel();

			fastPlayers[i]->calculated = false;
		}
	}
}
void FastIC::reset()
{
	oppMinDist = 1000.f;
	tmmMinDist = 1000.f;
	maxTackleProb = 0.0;
	maxReachProb = 0.0;
	outFlag = false;
	outCycle = 0;
	clearFastPlayers();
	fastPlayers.clear();
	clearFastestPlayers();
	fastestPlayers.clear();

}
void FastIC::calculate(bool debug)
{

	clearFastestPlayers();
	fastestPlayers.clear();
	FastBall tmpBall = ball;
	FastBall probBall = ball;

	Vector2D lastBallPos = ball.pos;

#ifdef FastICLOG
	dlog.addText(Logger::TEAM, __FILE__" first ball pos (%f , %f)", tmpBall.pos.x, tmpBall.pos.y);
	dlog.addText(Logger::TEAM, __FILE__" first ball vel %f", tmpBall.vel.length());
	dlog.addText(Logger::TEAM, __FILE__" first ball decay %f", tmpBall.decay);
	dlog.flush();
#endif
	int cycle = 0, firstReach = 9999, firstOppReach = 9999;

	float reachBeforeOthersProb = 1.0;

	outFlag = false;
	while (true)
	{
#ifdef FastICLOG
		dlog.addText(Logger::TEAM, __FILE__" cycle simulating is %i", cycle);
		dlog.addText(Logger::TEAM, __FILE__" first reach  is %i", firstReach);
		dlog.addText(Logger::TEAM, __FILE__" ball pos (%f , %f)", tmpBall.pos.x, tmpBall.pos.y);
		dlog.addText(Logger::TEAM, __FILE__" ball vel (%f , %f)", tmpBall.vel.x, tmpBall.vel.y);
#endif

		float probPrud = 1.0;

		for (int i = 0; i < fastPlayers.size(); i++)
		{
#ifdef FastICLOG
			dlog.addText(Logger::TEAM, __FILE__" checking player number %i",
					fastPlayers[i]->playerPtr->unum());
#endif
			if (fastPlayers[i]->calculated and !probMode)
			{
#ifdef FastICLOG
				dlog.addText(Logger::TEAM, __FILE__"this player calculated");
#endif
				continue;
			}

			/////////////////////

			float dashDist, distToBall, directDashDist;
			float dashDistP, distToBallP, directDashDistP;
			int turnCycle, turnCycleP;

			calcDashDist(cycle, tmpBall, fastPlayers[i], distToBall, dashDist, directDashDist, turnCycle);

			if (probMode)
				calcDashDist(cycle, probBall, fastPlayers[i], distToBallP, dashDistP, directDashDistP, turnCycleP);

			/////////////////////

			float thisKickable = fastPlayers[i]->playerPtr->playerTypePtr()->kickableArea();
			const double penalty_x_abs = ServerParam::i().pitchHalfLength() - ServerParam::i().penaltyAreaLength();
			const double penalty_y_abs = ServerParam::i().penaltyAreaHalfWidth();

			if (tmpBall.pos.absX() > penalty_x_abs and tmpBall.pos.absY() < penalty_y_abs and fastPlayers[i]->playerPtr->goalie())
				thisKickable = ServerParam::i().catchableArea() + 0.15;
			if (cycle > fastPlayers[i]->delay)
				thisKickable *= fastPlayers[i]->controlMulti;

#ifdef FastICLOG
			dlog.addText(Logger::TEAM, __FILE__" player turn cycle %i", turnCycle);
			dlog.addText(Logger::TEAM, __FILE__" player type is %i",
					fastPlayers[i]->playerPtr->type());
			dlog.addText(Logger::TEAM, __FILE__" player dash dist %f", dashDist);
			dlog.addText(Logger::TEAM, __FILE__" player kickable %f", thisKickable);
#endif
			if (debug)
				dlog.addCircle(Logger::PASS, fastPlayers[i]->pos, double(thisKickable + dashDist), min_max(0, (i <= 8) ? (i * 30) : 0, 255),
						min_max(0, (i <= 16) ? ((i - 8) * 30) : 0, 255), min_max(0, (i - 16) * 30, 255), false);
// _______________________________________________________________________________________________________________________
//|_________________________________________->		Probability Calcuting		<-_________________________________________|

			if (probMode)
			{
				float playerRand = (
						!(fastPlayers[i]->playerPtr->type() >= 0 && fastPlayers[i]->playerPtr->type() <= 17) ?
								ServerParam::i().defaultRealSpeedMax() : fastPlayers[i]->playerPtr->playerTypePtr()->realSpeedMax())
						* ServerParam::i().playerRand(), ballRand = ServerParam::i().ballRand() * ServerParam::i().ballSpeedMax();

				fastPlayers[i]->tempProb = 1.0
						- (calculateReachProb(float(cycle + 1), ((dashDistP + thisKickable) - distToBallP), playerRand + ballRand));
				probPrud *= fastPlayers[i]->tempProb;
			}
// ________________________________________________________________________________________________________________________
//|________________________________________________________________________________________________________________________|

			if (!outFlag and firstReach == 9999 and fastPlayers[i]->playerPtr->side() == wm.ourSide())
				tmmMinDist = min(tmmMinDist, distToBall - dashDist);
			if (!outFlag and firstReach == 9999 and fastPlayers[i]->playerPtr->side() != wm.ourSide())
			{

				oppMinDist = min(oppMinDist, distToBall - dashDist);

				double distance = distToBall - directDashDist;

				if (distance < ServerParam::i().tackleDist() and fastPlayers[i]->playerPtr->unum() != wm.theirGoalieUnum())
				{
					double tackle_fail_prob = 0.0;
					if (turnCycle == 0)
					{
						Vector2D oppMove;
						oppMove.setPolar(directDashDist, fastPlayers[i]->playerPtr->body());
						Vector2D opponentPos = fastPlayers[i]->playerPtr->pos() + oppMove;
						double yDist = (opponentPos - tmpBall.pos).absY();
						tackle_fail_prob = std::pow(distance / ServerParam::i().tackleDist(), ServerParam::i().tackleExponent())
								+ std::pow(yDist / ServerParam::i().tackleWidth(), ServerParam::i().tackleExponent());
					}
					else
					{
						tackle_fail_prob = (std::pow(distance / ServerParam::i().tackleDist(), ServerParam::i().tackleExponent()));
					}

					maxTackleProb = max(maxTackleProb, 1.0 - tackle_fail_prob);
				}
			}

			if (!fastPlayers[i]->calculated and dashDist + thisKickable > distToBall)
			{
				fastPlayers[i]->calculated = true;
				if (firstReach == 9999)
					firstReach = cycle;
				if (firstOppReach == 9999 and fastPlayers[i]->playerPtr->side() == wm.theirSide())
					firstOppReach = cycle;
				fastestPlayers.push_back(
						new FastestPlayer(fastPlayers[i]->playerPtr->unum(), fastPlayers[i]->playerPtr->side() == wm.ourSide(), cycle, tmpBall.pos,
								fastPlayers[i]->playerPtr, fastPlayers[i]->fake));
				fastPlayers[i]->resultPlayer = fastestPlayers.back();

				tmpBall.vel = Vector2D(0, 0);
			}

			if (!shootMode and fastPlayers[i]->playerPtr->side() != wm.ourSide() and !fastPlayers[i]->calculated
					and (cycle - ball.delay) > 0 /*and distToBall < (dashDist + thisKickable) * 1.25*/)
			{
				Segment2D ballMove(tmpBall.pos, lastBallPos);

				double distToLine = ballMove.dist(fastPlayers[i]->pos);

				if (distToLine < dashDist + thisKickable)
				{
					fastPlayers[i]->calculated = true;
					if (firstReach == 9999)
						firstReach = cycle;
					if (firstOppReach == 9999 and fastPlayers[i]->playerPtr->side() == wm.theirSide())
						firstOppReach = cycle;
					fastestPlayers.push_back(
							new FastestPlayer(fastPlayers[i]->playerPtr->unum(), fastPlayers[i]->playerPtr->side() == wm.ourSide(), cycle,
									tmpBall.pos, fastPlayers[i]->playerPtr, fastPlayers[i]->fake));
					fastPlayers[i]->resultPlayer = fastestPlayers.back();

					tmpBall.vel = Vector2D(0, 0);
				}

			}

//			if (fastPlayers[i]->playerPtr->velCount() < 2)
//			{
//				fastPlayers[i]->pos += fastPlayers[i]->vel;
//				fastPlayers[i]->vel *= fastPlayers[i]->playerPtr->playerTypePtr()->playerDecay();
//			}
			if (cycle == 1)
			{
				if (!fastPlayers[i]->playerPtr->isSelf())
					fastPlayers[i]->pos += fastPlayers[i]->vel / fastPlayers[i]->playerPtr->playerTypePtr()->playerDecay();
				else
				{
					fastPlayers[i]->pos += fastPlayers[i]->vel;
				}
			}

		}

		if (probMode)
		{
			reachBeforeOthersProb *= probPrud;

			for (int i = 0; i < fastPlayers.size(); i++)
			{
				fastPlayers[i]->mainProb += reachBeforeOthersProb * ((1.0 - fastPlayers[i]->tempProb) / fastPlayers[i]->tempProb);

//				dlog.addText(Logger::SHOOT,"CYCLE (%i) PLAYER NUM (%i) PLAYER  MAIN PROB (%f) PORB(%.2f)"
//						,cycle,fastPlayers[i]->playerPtr->unum(),fastPlayers[i]->mainProb,fastPlayers[i]->tempProb);

				if (fastPlayers[i]->resultPlayer)
					fastPlayers[i]->resultPlayer->reachProb = fastPlayers[i]->mainProb;
				maxReachProb = max(maxReachProb, fastPlayers[i]->mainProb);

			}
		}
		if (cycle >= ball.delay)
		{
			lastBallPos = tmpBall.pos;
			ballSimulate(&tmpBall);
			ballSimulate(&probBall);
		}
		if (cycle - firstReach > cyclesAfter || cycle > maxCycle || cycle - firstOppReach > maxOppCycle)
		{
			break;
		}
		float pitchLength = ServerParam::i().pitchHalfLength();
		float pitchWidth = ServerParam::i().pitchHalfWidth();
		if (!outFlag
				and (!Rect2D(Vector2D(-pitchLength, -pitchWidth), Vector2D(pitchLength, pitchWidth)).contains(tmpBall.pos)
						or (probMode and !Rect2D(Vector2D(-pitchLength, -pitchWidth), Vector2D(pitchLength, pitchWidth)).contains(probBall.pos))))
		{
			outFlag = true;
			outCycle = cycle;

			if (!shootMode)
				break;
		}
		cycle++;
	}
	if (debug)
	{
//#ifdef FastICLOG
		for (int i = 0; i < fastestPlayers.size(); i++)
		{
			dlog.addText(Logger::TEAM, __FILE__" FAST PLAYER NUMBER %i is player num %i team %i", i + 1, fastestPlayers[i]->unum,
					fastestPlayers[i]->isTeammate);
			dlog.addText(Logger::TEAM, __FILE__" FAST PLAYER NUMBER %i pos (%f %f)", i + 1, fastestPlayers[i]->playerPtr->pos().x,
					fastestPlayers[i]->playerPtr->pos().y);
			dlog.addText(Logger::TEAM, __FILE__" FAST PLAYER NUMBER %i reach cycle %i", i + 1, fastestPlayers[i]->reachCycle);
			dlog.addText(Logger::TEAM, __FILE__" FAST PLAYER NUMBER %i is receive point is %f %f", i + 1, fastestPlayers[i]->receivePoint.x,
					fastestPlayers[i]->receivePoint.y);
			dlog.addText(Logger::TEAM, __FILE__" FAST PLAYER NUMBER %i is PROB %f", i + 1, fastestPlayers[i]->reachProb);
		}
//#endif
	}
}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//input functions
void FastIC::setMaxCycleAfterFirstFastestPlayer(unsigned max)
{
	cyclesAfter = max;
#ifdef FastICLOG
	dlog.addText(Logger::TEAM, "cycle after set to %i", cyclesAfter);
#endif
}
void FastIC::setMaxCycles(unsigned max)
{
	maxCycle = max;
}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//out functions
bool FastIC::isSelfFastestPlayer() const
{
	if (fastestPlayers.size() > 0)
		return fastestPlayers[0]->playerPtr->isSelf();
	return false;
}
bool FastIC::isSelfFastestTeammate() const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (fastestPlayers[i]->isTeammate)
			return fastestPlayers[i]->playerPtr->isSelf();
	return false;
}
vector<FastestPlayer> FastIC::getFastestTeammates() const
{
	vector<FastestPlayer> teammates;
	teammates.clear();
	for (int i = 0; i < fastestPlayers.size(); i++)
	{
		if (fastestPlayers[i]->isTeammate)
		{
			teammates.push_back(*fastestPlayers[i]);
		}
	}
	return teammates;
}
vector<FastestPlayer> FastIC::getFastestOpponents() const
{
	vector<FastestPlayer> opponents;
	opponents.clear();
	for (int i = 0; i < fastestPlayers.size(); i++)
	{
		if (!fastestPlayers[i]->isTeammate)
		{
			opponents.push_back(*fastestPlayers[i]);
		}
	}
	return opponents;
}
int FastIC::getFastestTeammateReachCycle(bool withMe) const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (fastestPlayers[i]->isTeammate)
		{
			if (!withMe && fastestPlayers[i]->playerPtr->isSelf())
				continue;
			return fastestPlayers[i]->reachCycle;
		}

	return 9999;
}
Vector2D FastIC::getFastestPlayerReachPoint() const
{
	if (fastestPlayers.size() > 0)
		return fastestPlayers[0]->receivePoint;

	return Vector2D(1000, 1000);
}
Vector2D FastIC::getFastestTeammateReachPoint(bool withMe) const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (fastestPlayers[i]->isTeammate)
		{
			if (!withMe && fastestPlayers[i]->playerPtr->isSelf())
				continue;
			return fastestPlayers[i]->receivePoint;
		}

	return Vector2D(1000, 1000);
}
int FastIC::getFastestOpponentReachCycle() const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (!fastestPlayers[i]->isTeammate)
		{
			return fastestPlayers[i]->reachCycle;
		}

	return 9999;
}
Vector2D FastIC::getFastestOpponentReachPoint() const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (!fastestPlayers[i]->isTeammate)
		{
			return fastestPlayers[i]->receivePoint;
		}

	return Vector2D(1000.0, 1000.0);

}

int FastIC::getFastestPlayerReachCycle() const
{
	if (fastestPlayers.size() > 0)
		return fastestPlayers[0]->reachCycle;
	return 9999;

}
int FastIC::getSelfReachCycle() const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (fastestPlayers[i]->playerPtr->isSelf())
		{
			return fastestPlayers[i]->reachCycle;
		}

	return 9999;

}
const AbstractPlayerObject *
FastIC::getFastestTeammate(bool withMe) const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (fastestPlayers[i]->isTeammate)
		{
			return fastestPlayers[i]->playerPtr;

		}

	return NULL;

}
const AbstractPlayerObject *
FastIC::getFastestOpponent() const
{
	for (int i = 0; i < fastestPlayers.size(); i++)
		if (!fastestPlayers[i]->isTeammate)
		{
			return fastestPlayers[i]->playerPtr;

		}

	return NULL;

}
const AbstractPlayerObject *
FastIC::getFastestPlayer() const
{
	if (fastestPlayers.size() > 0)
		return fastestPlayers[0]->playerPtr;
	return NULL;
}
const vector<FastestPlayer*> FastIC::getFastestPlayers() const
{
	return fastestPlayers;
}

float FastIC::getFastestPlayerReachProb() const
{
	if (fastestPlayers.size() > 0)
		return fastestPlayers[0]->reachProb;
	return 0.0;

}

