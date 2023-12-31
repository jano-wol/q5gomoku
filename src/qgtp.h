#ifndef QGTP_H
#define QGTP_H

#include <QProcess>

#include "goboard.h"
#include "gogame.h"
#include "goeval.h"
#include "setting.h"
#include "textview.h"

struct time_settings;

class GTP_Process;

class GTP_Controller
{
	QWidget *m_parent;

protected:
	QMap<GTP_Process *, analyzer_id> m_id_map;

	GTP_Controller (QWidget *p) : m_parent (p) { }
	~GTP_Controller () = default;
	GTP_Process *create_gtp (const Engine &engine, int size_x, int size_y, double komi, bool show_dialog = true);
	GTP_Process *create_gtp (const Engine &engine, int size, double komi, bool show_dialog = true)
	{
		return create_gtp (engine, size, size, komi, show_dialog);
	}
public:
	virtual void gtp_played_move (GTP_Process *p, int x, int y) = 0;
	virtual void gtp_played_pass (GTP_Process *p) = 0;
	virtual void gtp_played_resign (GTP_Process *p) = 0;
	virtual void gtp_report_score (GTP_Process *p, const QString &) = 0;
	virtual void gtp_startup_success (GTP_Process *p) = 0;
	virtual void gtp_setup_success (GTP_Process *p) = 0;
	virtual void gtp_exited (GTP_Process *p) = 0;
	virtual void gtp_failure (GTP_Process *p, const QString &) = 0;
	virtual void gtp_eval (GTP_Process *, const QString &, bool)
	{
	}
	virtual void gtp_switch_ready () { }
};

enum class analyzer { disconnected, starting, running, paused };

class GTP_Eval_Controller : public GTP_Controller
{
	bool m_last_request_flipped {};
	go_game_ptr m_eval_game {};

protected:
	using GTP_Controller::GTP_Controller;
	~GTP_Eval_Controller();
	GTP_Process *m_analyzer {};
	double m_analyzer_komi = 0;

	bool m_pause_eval = false;
	/* Set if we are in the process of changing positions to analyze.  We send
	   a request to stop analyzing the current position to the GTP process and
	   set this variable.  It is cleared when a response arrives.
	   This solves the problem of receiving updates for an old position.  */
	bool m_switch_pending = false;
	/* Set if we should continue to receive updates, but do not want to update
	   the evaluation data.  Used by the board display to pause when the user
	   holds the right button.  */
	bool m_pause_updates = false;

	bool m_primary_have_score;
	double m_primary_eval;
	double m_primary_score;

	game_state *m_eval_state {};

	void clear_eval_data ();

	void start_analyzer (const Engine &engine, int size, double komi, bool show_dialog = true, int size_y_opt = -1);
	void stop_analyzer ();
	void pause_eval_updates (bool on) { m_pause_updates = on; }
	bool pause_analyzer ();
	void initiate_switch ();
	void set_analysis_state (go_game_ptr, game_state *);
	void setup_for_analysis (go_game_ptr, game_state *, bool flip = false);
	void request_analysis (go_game_ptr, game_state *, bool flip = false);
	virtual void eval_received (const analyzer_id &, const QString &, int, bool) = 0;
	virtual void analyzer_state_changed () { }
	virtual void notice_analyzer_id (const analyzer_id &, bool) { }
public:
	analyzer analyzer_state ();

	virtual void gtp_played_move (GTP_Process *, int, int) override { /* Should not happen.  */ }
	virtual void gtp_played_resign (GTP_Process *) override { /* Should not happen.  */ }
	virtual void gtp_played_pass (GTP_Process *) override { /* Should not happen.  */ }
	virtual void gtp_setup_success (GTP_Process *) override { /* Should not happen.  */ }
	virtual void gtp_report_score (GTP_Process *, const QString &) override { /* Should not happen.  */ }
	virtual void gtp_eval (GTP_Process *, const QString &, bool) override;
	virtual void gtp_switch_ready () override;
};

class GTP_Process : public QProcess
{
	Q_OBJECT

	QString m_name;

	QString m_buffer;
	QString m_stderr_buffer;

	TextView m_dlg;
	GTP_Controller *m_controller;

	int m_dlg_lines = 0;
	int m_size_x, m_size_y, m_size_y_rectangle_fix;
	/* The komi we've requested with the "komi" command.  The engine may have
	   ignored it.  */
	double m_komi;

	/* A tree in which we keep track of what the move history, in sync with the GTP engine.  */
	game_state_manager m_manager;
	game_state *m_moves {};
	game_state *m_last_move {};

	bool m_started = false;
	bool m_stopped = false;

	bool m_analyze_lz = false;
	bool m_analyze_kata = false;

	stone_color m_genmove_col = none;

	typedef void (GTP_Process::*t_receiver) (const QString &);
	QMap <int, t_receiver> m_receivers;
	QMap <int, t_receiver> m_err_receivers;
	QMap <int, t_receiver> m_end_receivers;
	t_receiver m_cur_receiver = nullptr;
	t_receiver m_end_receiver = nullptr;
	bool m_within_reply = false;

	/* Number of the next request.  */
	int req_cnt;
	void send_request(const QString &, t_receiver = nullptr, t_receiver = &GTP_Process::default_err_receiver);

	void startup_part2 (const QString &);
	void startup_part3 (const QString &);
	void startup_part4 (const QString &);
	void startup_part5 (const QString &);
	void startup_part6 (const QString &);
	void startup_part7 (const QString &);
	void setup_success (const QString &);
	void receive_move (const QString &);
	void pause_callback (const QString &);
	void receive_eval (const QString &);
	void score_callback_1 (const QString &);
	void score_callback_2 (const QString &);
	void internal_quit ();
	void default_err_receiver (const QString &);
	void rect_board_err_receiver (const QString &);
	void dup_move (game_state *, bool);
	void append_text (const QString &, const QColor &col);

public slots:
	void slot_started ();
	void slot_finished (int exitcode, QProcess::ExitStatus status);
	void slot_error (QProcess::ProcessError);
	void slot_receive_stdout ();
	void slot_startup_messages ();
	void slot_abort_request (bool);

public:
	GTP_Process (QWidget *parent, GTP_Controller *c, const Engine &engine,
		     int size_x, int size_y, float komi, bool show_dialog = true);
	~GTP_Process ();
	bool started () { return m_started; }
	bool stopped () { return m_stopped; }

	void clear_board () { send_request ("clear_board"); }
	void setup_board (game_state *, double, bool);
	bool setup_timing (const time_settings &);
	void send_remaining_time (stone_color col, const QString &);
	void setup_initial_position (game_state *);
	void request_move (stone_color col, bool);
	void request_score ();
	void played_move (stone_color col, int x, int y);
	void undo_move ();
	void komi (double);
	void played_move_pass (stone_color col);
	void played_move_resign (stone_color col);
	void analyze (stone_color col, int interval);
	void pause_analysis ();
	void initiate_analysis_switch ();

	void quit ();

	QDialog *dialog () { return &m_dlg; }
};

#endif
