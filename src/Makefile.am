## Process this file with automake to produce Makefile.in

UTILSSOURCES = \
    utils/rcsc_utils.h \
    utils/rcsc_utils.cpp \
    utils/geo_utils.h \
    utils/geo_utils.cpp \
    utils/algo_utils.h \
    utils/algo_utils.cpp \
    utils/algorithms/hungarian.h \
    utils/algorithms/hungarian.cpp


BHV_SOURCE = \
    behaviours/bhv_basic_move.cpp \
	behaviours/bhv_basic_offensive_kick.cpp \
	behaviours/bhv_basic_tackle.cpp \
	behaviours/bhv_custom_before_kick_off.cpp \
	behaviours/bhv_go_to_static_ball.cpp \
	behaviours/bhv_goalie_basic_move.cpp \
	behaviours/bhv_goalie_chase_ball.cpp \
	behaviours/bhv_goalie_free_kick.cpp \
	behaviours/bhv_prepare_set_play_kick.cpp \
	behaviours/bhv_set_play.cpp \
	behaviours/bhv_set_play_free_kick.cpp \
	behaviours/bhv_set_play_goal_kick.cpp \
	behaviours/bhv_set_play_indirect_free_kick.cpp \
	behaviours/bhv_set_play_kick_in.cpp \
	behaviours/bhv_set_play_kick_off.cpp \
	behaviours/bhv_their_goal_kick_move.cpp \
	behaviours/bhv_penalty_kick.cpp \
    behaviours/bhv_basic_move.h \
    behaviours/bhv_basic_offensive_kick.h \
    behaviours/bhv_basic_tackle.h \
    behaviours/bhv_custom_before_kick_off.h \
    behaviours/bhv_go_to_static_ball.h \
    behaviours/bhv_goalie_basic_move.h \
    behaviours/bhv_goalie_chase_ball.h \
    behaviours/bhv_goalie_free_kick.h \
    behaviours/bhv_prepare_set_play_kick.h \
    behaviours/bhv_set_play.h \
   	behaviours/bhv_set_play_free_kick.h \
   	behaviours/bhv_set_play_goal_kick.h \
   	behaviours/bhv_set_play_indirect_free_kick.h \
   	behaviours/bhv_set_play_kick_in.h \
   	behaviours/bhv_set_play_kick_off.h \
    behaviours/bhv_their_goal_kick_move.h \
    behaviours/bhv_penalty_kick.h \
    behaviours/bhv_defensive_positioning.h \
    behaviours/bhv_defensive_positioning.cpp \
    behaviours/bhv_defhalf_positioning.h \
    behaviours/bhv_defhalf_positioning.cpp \
    behaviours/bhv_block.h \
    behaviours/bhv_block.cpp



ROLE_SOURCE = \
    roles/soccer_role.h \
    roles/role_center_back.h \
    roles/role_center_forward.h \
    roles/role_defensive_half.h \
    roles/role_goalie.h \
    roles/role_offensive_half.h \
    roles/role_sample.h \
    roles/role_side_back.h \
    roles/role_side_forward.h \
    roles/role_side_half.h \
    roles/role_keepaway_keeper.h \
    roles/role_keepaway_taker.h \
    roles/soccer_role.cpp \
    roles/role_center_back.cpp \
    roles/role_center_forward.cpp \
    roles/role_defensive_half.cpp \
    roles/role_goalie.cpp \
    roles/role_offensive_half.cpp \
    roles/role_sample.cpp \
    roles/role_side_back.cpp \
    roles/role_side_forward.cpp \
    roles/role_side_half.cpp \
    roles/role_keepaway_keeper.cpp \
    roles/role_keepaway_taker.cpp

COMM_SOURCE = \
    communication/communication.h \
	communication/sample_communication.h \
	communication/keepaway_communication.h \
	communication/intention_receive.h \
    communication/intention_wait_after_set_play_kick.h \
    communication/sample_communication.cpp \
    communication/keepaway_communication.cpp \
    communication/intention_receive.cpp \
    communication/intention_wait_after_set_play_kick.cpp

ACTIONS_SOURCE = \
	actions/neck_default_intercept_neck.h \
	actions/neck_goalie_turn_neck.h \
	actions/neck_offensive_intercept_neck.h \
	actions/view_tactical.h \
	actions/neck_default_intercept_neck.cpp \
	actions/neck_goalie_turn_neck.cpp \
	actions/neck_offensive_intercept_neck.cpp \
	actions/view_tactical.cpp

PLAYERSOURCES = \
    cafe_model.cpp \
	sample_field_evaluator.cpp \
	sample_player.cpp \
	strategy.cpp \
	main_player.cpp

PLAYERHEADERS = \
    cafe_model.h\
	sample_field_evaluator.h \
	sample_player.h \
	strategy.h

CHAINACTIONSOURCES = \
	chain_action/actgen_action_chain_length_filter.h \
	chain_action/actgen_cross.cpp \
	chain_action/actgen_cross.h \
	chain_action/actgen_direct_pass.cpp \
	chain_action/actgen_direct_pass.h \
	chain_action/actgen_self_pass.cpp \
	chain_action/actgen_self_pass.h \
	chain_action/actgen_shoot.cpp \
	chain_action/actgen_shoot.h \
	chain_action/actgen_short_dribble.cpp \
	chain_action/actgen_short_dribble.h \
	chain_action/actgen_simple_dribble.cpp \
	chain_action/actgen_simple_dribble.h \
	chain_action/actgen_strict_check_pass.cpp \
	chain_action/actgen_strict_check_pass.h \
	chain_action/action_chain_graph.cpp \
	chain_action/action_chain_graph.h \
	chain_action/action_chain_holder.cpp \
	chain_action/action_chain_holder.h \
	chain_action/action_generator.h \
	chain_action/action_state_pair.h \
	chain_action/bhv_chain_action.cpp \
	chain_action/bhv_chain_action.h \
	chain_action/bhv_normal_dribble.cpp \
	chain_action/bhv_normal_dribble.h \
	chain_action/bhv_pass_kick_find_receiver.cpp \
	chain_action/bhv_pass_kick_find_receiver.h \
	chain_action/bhv_strict_check_shoot.cpp \
	chain_action/bhv_strict_check_shoot.h \
	chain_action/body_force_shoot.cpp \
	chain_action/body_force_shoot.h \
	chain_action/clear_ball.cpp \
	chain_action/clear_ball.h \
	chain_action/clear_generator.cpp \
	chain_action/clear_generator.h \
	chain_action/cooperative_action.cpp \
	chain_action/cooperative_action.h \
	chain_action/cross_generator.cpp \
	chain_action/cross_generator.h \
	chain_action/dribble.cpp \
	chain_action/dribble.h \
	chain_action/field_analyzer.cpp \
	chain_action/field_analyzer.h \
	chain_action/field_evaluator.h \
	chain_action/hold_ball.cpp \
	chain_action/hold_ball.h \
	chain_action/neck_turn_to_receiver.cpp \
	chain_action/neck_turn_to_receiver.h \
	chain_action/pass.cpp \
	chain_action/pass.h \
	chain_action/pass_checker.h \
	chain_action/predict_ball_object.h \
	chain_action/predict_player_object.h \
	chain_action/predict_state.cpp \
	chain_action/predict_state.h \
	chain_action/self_pass_generator.cpp \
	chain_action/self_pass_generator.h \
	chain_action/shoot.cpp \
	chain_action/shoot.h \
	chain_action/shoot_generator.cpp \
	chain_action/shoot_generator.h \
	chain_action/short_dribble_generator.cpp \
	chain_action/short_dribble_generator.h \
	chain_action/simple_pass_checker.cpp \
	chain_action/simple_pass_checker.h \
	chain_action/strict_check_pass_generator.cpp \
	chain_action/strict_check_pass_generator.h \
	chain_action/tackle_generator.cpp \
	chain_action/tackle_generator.h

COACHSOURCES = \
	sample_coach.cpp \
	main_coach.cpp

COACHHEADERS = \
	sample_coach.h

TRAINERSOURCES = \
	sample_trainer.cpp \
	main_trainer.cpp

TRAINERHEADERS = \
	sample_trainer.h


noinst_PROGRAMS = sample_player sample_coach sample_trainer

noinst_DATA = \
	start.sh.in \
	keepaway.sh.in \
	train.sh.in \
	coach.conf \
	player.conf \
	team_logo.xpm \
	formations-dt/before-kick-off.conf \
	formations-dt/goal-kick-opp.conf \
	formations-dt/goal-kick-our.conf \
	formations-dt/goalie-catch-opp.conf \
	formations-dt/goalie-catch-our.conf \
	formations-dt/goalie-formation.conf \
	formations-dt/indirect-freekick-opp-formation.conf \
	formations-dt/indirect-freekick-our-formation.conf \
	formations-dt/kickin-our-formation.conf \
	formations-dt/normal-formation.conf \
	formations-dt/defense-formation.conf \
	formations-dt/offense-formation.conf \
	formations-dt/setplay-opp-formation.conf \
	formations-dt/setplay-our-formation.conf \
	formations-keeper/before-kick-off.conf \
	formations-keeper/goal-kick-opp.conf \
	formations-keeper/goal-kick-our.conf \
	formations-keeper/goalie-catch-opp.conf \
	formations-keeper/goalie-catch-our.conf \
	formations-keeper/goalie-formation.conf \
	formations-keeper/indirect-freekick-opp-formation.conf \
	formations-keeper/indirect-freekick-our-formation.conf \
	formations-keeper/kickin-our-formation.conf \
	formations-keeper/normal-formation.conf \
	formations-keeper/defense-formation.conf \
	formations-keeper/offense-formation.conf \
	formations-keeper/setplay-opp-formation.conf \
	formations-keeper/setplay-our-formation.conf \
	formations-taker/before-kick-off.conf \
	formations-taker/goal-kick-opp.conf \
	formations-taker/goal-kick-our.conf \
	formations-taker/goalie-catch-opp.conf \
	formations-taker/goalie-catch-our.conf \
	formations-taker/goalie-formation.conf \
	formations-taker/indirect-freekick-opp-formation.conf \
	formations-taker/indirect-freekick-our-formation.conf \
	formations-taker/kickin-our-formation.conf \
	formations-taker/normal-formation.conf \
	formations-taker/defense-formation.conf \
	formations-taker/offense-formation.conf \
	formations-taker/setplay-opp-formation.conf \
	formations-taker/setplay-our-formation.conf

sample_player_SOURCES = \
	$(CHAINACTIONSOURCES) \
	$(BHV_SOURCE) \
	$(ROLE_SOURCE) \
	$(ACTIONS_SOURCE)  \
	$(COMM_SOURCE) \
	$(UTILSSOURCES) \
	$(PLAYERSOURCES)


sample_player_CXXFLAGS = -W -Wall
sample_player_LDFLAGS =
sample_player_LDADD =

sample_coach_SOURCES = \
	$(COACHSOURCES)
sample_coach_CXXFLAGS = -W -Wall
sample_coach_LDFLAGS =
sample_coach_LDADD =

sample_trainer_SOURCES = \
	$(TRAINERSOURCES)
sample_trainer_CXXFLAGS = -W -Wall
sample_trainer_LDFLAGS =
sample_trainer_LDADD =

noinst_HEADERS = \
	$(PLAYERHEADERS) \
	$(COACHHEADERS) \
	$(TRAINERHEADERS)

AM_CPPFLAGS = -I$(top_srcdir)/src  -I$(top_srcdir)/src/chain_action
AM_CFLAGS = -W -Wall
AM_CXXFLAGS = -W -Wall
AM_LDLAGS =

CLEANFILES = core *~

DISTCLEANFILES = start.sh train.sh

EXTRA_DIST = $(noinst_DATA) \
	start-debug.sh \
	start-offline.sh
