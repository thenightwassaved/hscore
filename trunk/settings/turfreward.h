
/* turfreward.h  - settings definitions */
/* dist: public */

/* disable rewards */
#define TR_STYLE_DISABLED  0

/* reward style setttings */
/* simple periodic scoring */
/* (when you want stats or access to the additional flag info) */
#define TR_STYLE_PERIODIC  1

/* standard weighted scoring method */
#define TR_STYLE_STANDARD  2

/* number of weights = number of points awarded */
#define TR_STYLE_WEIGHTS   3

/* each team gets a fixed # of points based on 1st, 2nd, 3rd, ... place */
#define TR_STYLE_FIXED_PTS 4


/* weight calculation settings */
#define TR_WEIGHT_DINGS 0 /* weight calculation based on dings */
#define TR_WEIGHT_TIME  1 /* weight calculation based on time */


/* recovery system settings */
#define TR_RECOVERY_DINGS          0 /* recovery cutoff based on RecoverDings */
#define TR_RECOVERY_TIME           1 /*  recovery cutoff based on RecoverTime */
#define TR_RECOVERY_DINGS_AND_TIME 2 /* recovery cutoff based on both RecoverDings and RecoverTime */

