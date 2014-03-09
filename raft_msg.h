#ifndef RAFT_MSG_INCLUDED_H
#define RAFT_MSG_INCLUDED_H

class msg_requestvote_t {
    /* candidate's term */
    int d_term;

    /* candidate requesting vote */
    int d_candidate_id;

    /* idx of candidate's last log entry */
    int d_last_log_idx;

    /* term of candidate's last log entry */
    int d_last_log_term;

public:

    msg_requestvote_t(int term, int candidate_id, int last_log_idx, int last_log_term):
      d_term(term), d_candidate_id(candidate_id), d_last_log_idx(last_log_idx), d_last_log_term(last_log_term) {}

    inline int term() { return d_term; }
    inline int candidate_id() { return d_candidate_id; }
    inline int last_log_idx() { return d_last_log_idx; }
    inline int last_log_term() { return d_last_log_term; }

    inline void term(int term) { d_term = term; }
    inline void candidate_id(int candidate_id) { d_candidate_id = candidate_id; }
    inline void last_log_idx(int last_log_idx) { d_last_log_idx = last_log_idx; }
    inline void last_log_term(int last_log_term) { d_last_log_term = last_log_term; }

};

class msg_entry_t {
    /* the entry's unique ID */
    unsigned int d_id;

    /* entry data */
    unsigned char* d_data;

    /* length of entry data */
    unsigned int d_len;

public:

    msg_entry_t(): d_id(0), d_data(NULL), d_len(0) { }

    msg_entry_t(unsigned int id, unsigned char* data, unsigned int len):
    	d_id(id), d_data(data), d_len(len) { }

	unsigned char* data() const {
		return d_data;
	}

	void data(unsigned char* data) {
		this->d_data = data;
	}

	unsigned int id() const {
		return d_id;
	}

	void id(unsigned int id) {
		this->d_id = id;
	}

	unsigned int len() const {
		return d_len;
	}

	void len(unsigned int len) {
		this->d_len = len;
	}
};

typedef struct {
    /* the entry's unique ID */
    unsigned int id;

    /* whether or not the entry was committed */
    int was_committed;
} msg_entry_response_t;

typedef struct {
    /* currentTerm, for candidate to update itself */
    int term;

    /* true means candidate received vote */
    int vote_granted;
} msg_requestvote_response_t;

typedef struct {
    int term;
    int leader_id;
    int prev_log_idx;
    int prev_log_term;
    int n_entries;
    msg_entry_t* entries;
    int leader_commit;
} msg_appendentries_t;

typedef struct {
    /* currentTerm, for leader to update itself */
    int term;

    /* success true if follower contained entry matching
     * prevLogidx and prevLogTerm */
    int success;

    /* Non-Raft fields follow: */
    /* Having the following fields allows us to do less book keeping in
     * regards to full fledged RPC */
    /* This is the highest log IDX we've received and appended to our log */
    int current_idx;
    /* The first idx that we received within the appendentries message */
    int first_idx;
} msg_appendentries_response_t;

enum {
    RAFT_MSG_REQUESTVOTE,
    RAFT_MSG_REQUESTVOTE_RESPONSE,
    RAFT_MSG_APPENDENTRIES,
    RAFT_MSG_APPENDENTRIES_RESPONSE,
    RAFT_MSG_ENTRY,
    RAFT_MSG_ENTRY_RESPONSE,
};

#endif
