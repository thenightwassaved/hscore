
/* declarations */

struct bl_brick
{
	unsigned char x1, y1, x2, y2;
};

struct bl_letter
{
	int width, bricknum;
	struct bl_brick *bricks; /* can be null if bricknum == 0 */
};

static const int letterheight = 8;

/* letters */

/* ' ' */
/* no bricks */

/* '!' */
static struct bl_brick bl_letter_33[] =
{
	{ 1, 0, 1, 3 },
	{ 0, 0, 2, 0 },
	{ 0, 1, 2, 1 },
	{ 0, 2, 2, 2 },
	{ 0, 5, 2, 5 },
	{ 0, 6, 2, 6 },
};

/* '"' */
static struct bl_brick bl_letter_34[] =
{
	{ 0, 0, 2, 0 },
	{ 4, 0, 6, 0 },
	{ 0, 1, 2, 1 },
	{ 4, 1, 6, 1 },
	{ 1, 2, 1, 2 },
	{ 5, 2, 5, 2 },
};

/* '#' */
static struct bl_brick bl_letter_35[] =
{
	{ 0, 2, 6, 2 },
	{ 0, 4, 6, 4 },
	{ 2, 0, 2, 6 },
	{ 4, 0, 4, 6 },
};

/* '$' */
static struct bl_brick bl_letter_36[] =
{
	{ 3, 0, 3, 6 },
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 2 },
	{ 6, 4, 6, 5 },
	{ 6, 1, 6, 1 },
	{ 0, 5, 0, 5 },
};

/* '%' */
static struct bl_brick bl_letter_37[] =
{
	{ 0, 0, 2, 0 },
	{ 0, 2, 2, 2 },
	{ 4, 4, 6, 4 },
	{ 4, 6, 6, 6 },
	{ 0, 1, 0, 2 },
	{ 2, 1, 2, 2 },
	{ 4, 5, 4, 6 },
	{ 6, 5, 6, 6 },
	{ 6, 0, 6, 0 },
	{ 5, 1, 5, 1 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
	{ 2, 4, 2, 4 },
	{ 1, 5, 1, 5 },
	{ 0, 6, 0, 6 },
};

/* '&' */
static struct bl_brick bl_letter_38[] =
{
	{ 1, 3, 3, 3 },
	{ 1, 6, 3, 6 },
	{ 2, 0, 3, 0 },
	{ 2, 2, 3, 2 },
	{ 0, 4, 0, 5 },
	{ 1, 1, 1, 1 },
	{ 4, 1, 4, 1 },
	{ 4, 4, 4, 4 },
	{ 6, 4, 6, 4 },
	{ 5, 5, 5, 5 },
	{ 6, 6, 6, 6 },
};

/* ''' */
static struct bl_brick bl_letter_39[] =
{
	{ 0, 0, 2, 0 },
	{ 0, 1, 2, 1 },
	{ 1, 2, 1, 2 },
	{ 0, 3, 0, 3 },
};

/* '(' */
static struct bl_brick bl_letter_40[] =
{
	{ 0, 2, 0, 4 },
	{ 2, 0, 3, 0 },
	{ 2, 6, 3, 6 },
	{ 1, 1, 1, 1 },
	{ 1, 5, 1, 5 },
};

/* ')' */
static struct bl_brick bl_letter_41[] =
{
	{ 3, 2, 3, 4 },
	{ 0, 0, 1, 0 },
	{ 0, 6, 1, 6 },
	{ 2, 1, 2, 1 },
	{ 2, 5, 2, 5 },
};

/* '*' */
static struct bl_brick bl_letter_42[] =
{
	{ 0, 3, 6, 3 },
	{ 2, 2, 2, 4 },
	{ 4, 2, 4, 4 },
	{ 1, 1, 1, 1 },
	{ 5, 1, 5, 1 },
	{ 1, 5, 1, 5 },
	{ 5, 5, 5, 5 },
};

/* '+' */
static struct bl_brick bl_letter_43[] =
{
	{ 0, 3, 4, 3 },
	{ 2, 1, 2, 5 },
};

/* ',' */
static struct bl_brick bl_letter_44[] =
{
	{ 0, 4, 2, 4 },
	{ 0, 5, 2, 5 },
	{ 1, 6, 1, 6 },
	{ 0, 7, 0, 7 },
};

/* '-' */
static struct bl_brick bl_letter_45[] =
{
	{ 0, 3, 4, 3 },
};

/* '.' */
static struct bl_brick bl_letter_46[] =
{
	{ 0, 4, 2, 4 },
	{ 0, 5, 2, 5 },
	{ 0, 6, 2, 6 },
};

/* '/' */
static struct bl_brick bl_letter_47[] =
{
	{ 6, 0, 6, 0 },
	{ 5, 1, 5, 1 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
	{ 2, 4, 2, 4 },
	{ 1, 5, 1, 5 },
	{ 0, 6, 0, 6 },
};

/* '0' */
static struct bl_brick bl_letter_48[] =
{
	{ 2, 0, 4, 0 },
	{ 2, 6, 4, 6 },
	{ 0, 2, 0, 4 },
	{ 6, 2, 6, 4 },
	{ 1, 1, 1, 1 },
	{ 5, 1, 5, 1 },
	{ 1, 5, 1, 5 },
	{ 5, 5, 5, 5 },
};

/* '1' */
static struct bl_brick bl_letter_49[] =
{
	{ 3, 0, 3, 6 },
	{ 1, 6, 5, 6 },
	{ 2, 1, 3, 1 },
	{ 1, 2, 1, 2 },
};

/* '2' */
static struct bl_brick bl_letter_50[] =
{
	{ 0, 6, 6, 6 },
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 0, 4, 0, 6 },
	{ 6, 1, 6, 2 },
	{ 0, 1, 0, 1 },
};

/* '3' */
static struct bl_brick bl_letter_51[] =
{
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 1, 6, 5, 6 },
	{ 6, 1, 6, 2 },
	{ 6, 4, 6, 5 },
	{ 0, 1, 0, 1 },
	{ 0, 5, 0, 5 },
};

/* '4' */
static struct bl_brick bl_letter_52[] =
{
	{ 0, 4, 6, 4 },
	{ 5, 1, 5, 6 },
	{ 0, 0, 0, 4 },
};

/* '5' */
static struct bl_brick bl_letter_53[] =
{
	{ 0, 0, 6, 0 },
	{ 0, 3, 5, 3 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 3 },
	{ 6, 4, 6, 5 },
	{ 0, 5, 0, 5 },
};

/* '6' */
static struct bl_brick bl_letter_54[] =
{
	{ 0, 3, 5, 3 },
	{ 1, 0, 5, 0 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 5 },
	{ 6, 4, 6, 5 },
	{ 6, 1, 6, 1 },
};

/* '7' */
static struct bl_brick bl_letter_55[] =
{
	{ 0, 0, 6, 0 },
	{ 2, 4, 2, 6 },
	{ 0, 1, 0, 1 },
	{ 5, 1, 5, 1 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
};

/* '8' */
static struct bl_brick bl_letter_56[] =
{
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 2 },
	{ 0, 4, 0, 5 },
	{ 6, 1, 6, 2 },
	{ 6, 4, 6, 5 },
};

/* '9' */
static struct bl_brick bl_letter_57[] =
{
	{ 1, 3, 6, 3 },
	{ 1, 0, 5, 0 },
	{ 1, 6, 5, 6 },
	{ 6, 1, 6, 5 },
	{ 0, 1, 0, 2 },
	{ 0, 5, 0, 5 },
};

/* ':' */
static struct bl_brick bl_letter_58[] =
{
	{ 0, 1, 2, 1 },
	{ 0, 5, 2, 5 },
	{ 1, 0, 1, 2 },
	{ 1, 4, 1, 6 },
};

/* ';' */
static struct bl_brick bl_letter_59[] =
{
	{ 0, 1, 2, 1 },
	{ 0, 2, 2, 2 },
	{ 0, 4, 2, 4 },
	{ 0, 5, 2, 5 },
	{ 1, 6, 1, 6 },
	{ 0, 7, 0, 7 },
};

/* '<' */
static struct bl_brick bl_letter_60[] =
{
	{ 3, 0, 3, 0 },
	{ 2, 1, 2, 1 },
	{ 1, 2, 1, 2 },
	{ 0, 3, 0, 3 },
	{ 1, 4, 1, 4 },
	{ 2, 5, 2, 5 },
	{ 3, 6, 3, 6 },
};

/* '=' */
static struct bl_brick bl_letter_61[] =
{
	{ 0, 2, 4, 2 },
	{ 0, 4, 4, 4 },
};

/* '>' */
static struct bl_brick bl_letter_62[] =
{
	{ 0, 0, 0, 0 },
	{ 1, 1, 1, 1 },
	{ 2, 2, 2, 2 },
	{ 3, 3, 3, 3 },
	{ 2, 4, 2, 4 },
	{ 1, 5, 1, 5 },
	{ 0, 6, 0, 6 },
};

/* '?' */
static struct bl_brick bl_letter_63[] =
{
	{ 1, 0, 5, 0 },
	{ 3, 3, 5, 3 },
	{ 6, 1, 6, 2 },
	{ 0, 1, 0, 1 },
	{ 3, 4, 3, 4 },
	{ 3, 6, 3, 6 },
};

/* '@' */
static struct bl_brick bl_letter_64[] =
{
	{ 1, 0, 5, 0 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 5 },
	{ 2, 4, 5, 4 },
	{ 2, 2, 4, 2 },
	{ 2, 3, 4, 3 },
	{ 6, 1, 6, 3 },
};

/* 'A' */
static struct bl_brick bl_letter_65[] =
{
	{ 0, 4, 6, 4 },
	{ 0, 3, 0, 6 },
	{ 6, 3, 6, 6 },
	{ 3, 0, 3, 0 },
	{ 2, 1, 2, 1 },
	{ 4, 1, 4, 1 },
	{ 1, 2, 1, 2 },
	{ 5, 2, 5, 2 },
};

/* 'B' */
static struct bl_brick bl_letter_66[] =
{
	{ 0, 0, 0, 6 },
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 1, 6, 5, 6 },
	{ 6, 1, 6, 2 },
	{ 6, 4, 6, 5 },
};

/* 'C' */
static struct bl_brick bl_letter_67[] =
{
	{ 1, 0, 5, 0 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 5 },
	{ 6, 1, 6, 1 },
	{ 6, 5, 6, 5 },
};

/* 'D' */
static struct bl_brick bl_letter_68[] =
{
	{ 0, 0, 0, 6 },
	{ 1, 0, 5, 0 },
	{ 1, 6, 5, 6 },
	{ 6, 1, 6, 5 },
};

/* 'E' */
static struct bl_brick bl_letter_69[] =
{
	{ 0, 0, 6, 0 },
	{ 0, 6, 6, 6 },
	{ 0, 1, 0, 6 },
	{ 1, 3, 4, 3 },
};

/* 'F' */
static struct bl_brick bl_letter_70[] =
{
	{ 0, 0, 6, 0 },
	{ 0, 1, 0, 6 },
	{ 1, 3, 4, 3 },
};

/* 'G' */
static struct bl_brick bl_letter_71[] =
{
	{ 1, 0, 5, 0 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 5 },
	{ 3, 3, 6, 3 },
	{ 6, 4, 6, 5 },
	{ 6, 1, 6, 1 },
};

/* 'H' */
static struct bl_brick bl_letter_72[] =
{
	{ 0, 3, 6, 3 },
	{ 0, 0, 0, 6 },
	{ 6, 0, 6, 6 },
};

/* 'I' */
static struct bl_brick bl_letter_73[] =
{
	{ 1, 0, 1, 6 },
	{ 0, 0, 2, 0 },
	{ 0, 6, 2, 6 },
};

/* 'J' */
static struct bl_brick bl_letter_74[] =
{
	{ 6, 0, 6, 5 },
	{ 1, 6, 5, 6 },
	{ 0, 4, 0, 5 },
};

/* 'K' */
static struct bl_brick bl_letter_75[] =
{
	{ 0, 0, 0, 6 },
	{ 1, 3, 2, 3 },
	{ 5, 0, 5, 0 },
	{ 4, 1, 4, 1 },
	{ 3, 2, 3, 2 },
	{ 3, 4, 3, 4 },
	{ 4, 5, 4, 5 },
	{ 5, 6, 5, 6 },
};

/* 'L' */
static struct bl_brick bl_letter_76[] =
{
	{ 0, 6, 6, 6 },
	{ 0, 0, 0, 6 },
};

/* 'M' */
static struct bl_brick bl_letter_77[] =
{
	{ 0, 0, 0, 6 },
	{ 6, 0, 6, 6 },
	{ 5, 1, 6, 1 },
	{ 1, 1, 1, 1 },
	{ 2, 2, 2, 2 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
};

/* 'N' */
static struct bl_brick bl_letter_78[] =
{
	{ 0, 0, 0, 6 },
	{ 6, 0, 6, 6 },
	{ 5, 5, 6, 5 },
	{ 1, 1, 1, 1 },
	{ 2, 2, 2, 2 },
	{ 3, 3, 3, 3 },
	{ 4, 4, 4, 4 },
};

/* 'O' */
static struct bl_brick bl_letter_79[] =
{
	{ 0, 0, 6, 0 },
	{ 0, 6, 6, 6 },
	{ 0, 1, 0, 6 },
	{ 6, 1, 6, 6 },
};

/* 'P' */
static struct bl_brick bl_letter_80[] =
{
	{ 0, 0, 0, 6 },
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 6, 1, 6, 2 },
};

/* 'Q' */
static struct bl_brick bl_letter_81[] =
{
	{ 1, 0, 5, 0 },
	{ 0, 1, 0, 5 },
	{ 1, 6, 4, 6 },
	{ 6, 1, 6, 4 },
	{ 4, 4, 4, 4 },
	{ 5, 5, 5, 5 },
	{ 6, 6, 6, 6 },
};

/* 'R' */
static struct bl_brick bl_letter_82[] =
{
	{ 0, 0, 0, 6 },
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 6, 1, 6, 2 },
	{ 4, 4, 4, 4 },
	{ 5, 5, 5, 5 },
	{ 6, 6, 6, 6 },
};

/* 'S' */
static struct bl_brick bl_letter_83[] =
{
	{ 1, 0, 5, 0 },
	{ 1, 3, 5, 3 },
	{ 1, 6, 5, 6 },
	{ 0, 1, 0, 2 },
	{ 6, 4, 6, 5 },
	{ 6, 1, 6, 1 },
	{ 0, 5, 0, 5 },
};

/* 'T' */
static struct bl_brick bl_letter_84[] =
{
	{ 0, 0, 6, 0 },
	{ 3, 1, 3, 6 },
};

/* 'U' */
static struct bl_brick bl_letter_85[] =
{
	{ 0, 0, 0, 5 },
	{ 6, 0, 6, 5 },
	{ 1, 6, 5, 6 },
};

/* 'V' */
static struct bl_brick bl_letter_86[] =
{
	{ 0, 0, 0, 3 },
	{ 6, 0, 6, 3 },
	{ 1, 4, 1, 4 },
	{ 5, 4, 5, 4 },
	{ 2, 5, 2, 5 },
	{ 4, 5, 4, 5 },
	{ 3, 6, 3, 6 },
};

/* 'W' */
static struct bl_brick bl_letter_87[] =
{
	{ 0, 0, 0, 5 },
	{ 6, 0, 6, 5 },
	{ 3, 1, 3, 5 },
	{ 1, 6, 2, 6 },
	{ 4, 6, 5, 6 },
};

/* 'X' */
static struct bl_brick bl_letter_88[] =
{
	{ 0, 0, 0, 0 },
	{ 6, 0, 6, 0 },
	{ 1, 1, 1, 1 },
	{ 5, 1, 5, 1 },
	{ 2, 2, 2, 2 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
	{ 2, 4, 2, 4 },
	{ 4, 4, 4, 4 },
	{ 1, 5, 1, 5 },
	{ 5, 5, 5, 5 },
	{ 0, 6, 0, 6 },
	{ 6, 6, 6, 6 },
};

/* 'Y' */
static struct bl_brick bl_letter_89[] =
{
	{ 3, 3, 3, 6 },
	{ 0, 0, 0, 0 },
	{ 6, 0, 6, 0 },
	{ 1, 1, 1, 1 },
	{ 5, 1, 5, 1 },
	{ 2, 2, 2, 2 },
	{ 4, 2, 4, 2 },
};

/* 'Z' */
static struct bl_brick bl_letter_90[] =
{
	{ 0, 0, 6, 0 },
	{ 0, 6, 6, 6 },
	{ 1, 5, 1, 6 },
	{ 5, 1, 5, 1 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
	{ 2, 4, 2, 4 },
};

/* '[' */
static struct bl_brick bl_letter_91[] =
{
	{ 0, 0, 0, 6 },
	{ 1, 0, 4, 0 },
	{ 1, 6, 4, 6 },
};

/* '\' */
static struct bl_brick bl_letter_92[] =
{
	{ 0, 0, 0, 0 },
	{ 1, 1, 1, 1 },
	{ 2, 2, 2, 2 },
	{ 3, 3, 3, 3 },
	{ 4, 4, 4, 4 },
	{ 5, 5, 5, 5 },
	{ 6, 6, 6, 6 },
};

/* ']' */
static struct bl_brick bl_letter_93[] =
{
	{ 4, 0, 4, 6 },
	{ 0, 0, 4, 0 },
	{ 0, 6, 4, 6 },
};

/* '^' */
static struct bl_brick bl_letter_94[] =
{
	{ 2, 0, 2, 0 },
	{ 1, 1, 1, 1 },
	{ 3, 1, 3, 1 },
	{ 0, 2, 0, 2 },
	{ 4, 2, 4, 2 },
};

/* '_' */
static struct bl_brick bl_letter_95[] =
{
	{ 0, 7, 6, 7 },
};

/* '`' */
static struct bl_brick bl_letter_96[] =
{
	{ 0, 0, 2, 0 },
	{ 0, 1, 2, 1 },
	{ 1, 2, 1, 2 },
	{ 2, 3, 2, 3 },
};

/* 'a' */
static struct bl_brick bl_letter_97[] =
{
	{ 0, 4, 5, 4 },
	{ 0, 3, 0, 6 },
	{ 5, 3, 5, 6 },
	{ 2, 1, 3, 1 },
	{ 1, 2, 1, 2 },
	{ 4, 2, 4, 2 },
};

/* 'b' */
static struct bl_brick bl_letter_98[] =
{
	{ 0, 1, 0, 6 },
	{ 1, 1, 4, 1 },
	{ 1, 3, 4, 3 },
	{ 1, 6, 4, 6 },
	{ 5, 4, 5, 5 },
	{ 5, 2, 5, 2 },
};

/* 'c' */
static struct bl_brick bl_letter_99[] =
{
	{ 1, 1, 4, 1 },
	{ 1, 6, 4, 6 },
	{ 0, 2, 0, 5 },
	{ 5, 2, 5, 2 },
	{ 5, 5, 5, 5 },
};

/* 'd' */
static struct bl_brick bl_letter_100[] =
{
	{ 0, 1, 0, 6 },
	{ 1, 1, 4, 1 },
	{ 1, 6, 4, 6 },
	{ 5, 2, 5, 5 },
};

/* 'e' */
static struct bl_brick bl_letter_101[] =
{
	{ 0, 1, 5, 1 },
	{ 0, 6, 5, 6 },
	{ 0, 3, 4, 3 },
	{ 0, 2, 0, 6 },
};

/* 'f' */
static struct bl_brick bl_letter_102[] =
{
	{ 0, 1, 5, 1 },
	{ 0, 3, 4, 3 },
	{ 0, 2, 0, 6 },
};

/* 'g' */
static struct bl_brick bl_letter_103[] =
{
	{ 1, 1, 4, 1 },
	{ 1, 6, 4, 6 },
	{ 0, 2, 0, 5 },
	{ 3, 4, 5, 4 },
	{ 5, 2, 5, 2 },
	{ 5, 5, 5, 5 },
};

/* 'h' */
static struct bl_brick bl_letter_104[] =
{
	{ 0, 3, 5, 3 },
	{ 0, 1, 0, 6 },
	{ 5, 1, 5, 6 },
};

/* 'i' */
static struct bl_brick bl_letter_105[] =
{
	{ 0, 1, 0, 6 },
};

/* 'j' */
static struct bl_brick bl_letter_106[] =
{
	{ 5, 1, 5, 5 },
	{ 1, 6, 4, 6 },
	{ 0, 5, 0, 5 },
};

/* 'k' */
static struct bl_brick bl_letter_107[] =
{
	{ 0, 1, 0, 6 },
	{ 1, 3, 3, 3 },
	{ 5, 1, 5, 1 },
	{ 4, 2, 4, 2 },
	{ 3, 4, 3, 4 },
	{ 4, 5, 4, 5 },
	{ 5, 6, 5, 6 },
};

/* 'l' */
static struct bl_brick bl_letter_108[] =
{
	{ 0, 6, 5, 6 },
	{ 0, 1, 0, 6 },
};

/* 'm' */
static struct bl_brick bl_letter_109[] =
{
	{ 0, 1, 0, 6 },
	{ 5, 1, 5, 6 },
	{ 4, 2, 5, 2 },
	{ 2, 3, 3, 3 },
	{ 1, 2, 1, 2 },
};

/* 'n' */
static struct bl_brick bl_letter_110[] =
{
	{ 0, 1, 0, 6 },
	{ 5, 1, 5, 6 },
	{ 4, 5, 5, 5 },
	{ 1, 2, 1, 2 },
	{ 2, 3, 2, 3 },
	{ 3, 4, 3, 4 },
};

/* 'o' */
static struct bl_brick bl_letter_111[] =
{
	{ 1, 1, 4, 1 },
	{ 1, 6, 4, 6 },
	{ 0, 2, 0, 5 },
	{ 5, 2, 5, 5 },
};

/* 'p' */
static struct bl_brick bl_letter_112[] =
{
	{ 0, 1, 0, 6 },
	{ 1, 1, 4, 1 },
	{ 1, 4, 4, 4 },
	{ 5, 2, 5, 3 },
};

/* 'q' */
static struct bl_brick bl_letter_113[] =
{
	{ 1, 1, 4, 1 },
	{ 0, 2, 0, 5 },
	{ 1, 6, 3, 6 },
	{ 5, 2, 5, 4 },
	{ 3, 4, 3, 4 },
	{ 4, 5, 4, 5 },
	{ 5, 6, 5, 6 },
};

/* 'r' */
static struct bl_brick bl_letter_114[] =
{
	{ 0, 1, 0, 6 },
	{ 1, 1, 4, 1 },
	{ 1, 4, 4, 4 },
	{ 5, 2, 5, 3 },
	{ 4, 5, 4, 5 },
	{ 5, 6, 5, 6 },
};

/* 's' */
static struct bl_brick bl_letter_115[] =
{
	{ 1, 1, 4, 1 },
	{ 1, 3, 4, 3 },
	{ 1, 6, 4, 6 },
	{ 5, 4, 5, 5 },
	{ 0, 2, 0, 2 },
	{ 0, 5, 0, 5 },
};

/* 't' */
static struct bl_brick bl_letter_116[] =
{
	{ 2, 1, 2, 6 },
	{ 0, 1, 4, 1 },
};

/* 'u' */
static struct bl_brick bl_letter_117[] =
{
	{ 0, 1, 0, 5 },
	{ 5, 1, 5, 5 },
	{ 1, 6, 4, 6 },
};

/* 'v' */
static struct bl_brick bl_letter_118[] =
{
	{ 0, 1, 0, 4 },
	{ 5, 1, 5, 4 },
	{ 2, 6, 3, 6 },
	{ 1, 5, 1, 5 },
	{ 4, 5, 4, 5 },
};

/* 'w' */
static struct bl_brick bl_letter_119[] =
{
	{ 0, 1, 0, 6 },
	{ 5, 1, 5, 6 },
	{ 2, 4, 3, 4 },
	{ 4, 5, 5, 5 },
	{ 1, 5, 1, 5 },
};

/* 'x' */
static struct bl_brick bl_letter_120[] =
{
	{ 2, 3, 3, 3 },
	{ 2, 4, 3, 4 },
	{ 0, 1, 0, 1 },
	{ 5, 1, 5, 1 },
	{ 1, 2, 1, 2 },
	{ 4, 2, 4, 2 },
	{ 1, 5, 1, 5 },
	{ 4, 5, 4, 5 },
	{ 0, 6, 0, 6 },
	{ 5, 6, 5, 6 },
};

/* 'y' */
static struct bl_brick bl_letter_121[] =
{
	{ 2, 3, 2, 6 },
	{ 0, 1, 0, 1 },
	{ 4, 1, 4, 1 },
	{ 1, 2, 1, 2 },
	{ 3, 2, 3, 2 },
};

/* 'z' */
static struct bl_brick bl_letter_122[] =
{
	{ 0, 1, 5, 1 },
	{ 0, 6, 5, 6 },
	{ 1, 5, 1, 6 },
	{ 4, 2, 4, 2 },
	{ 3, 3, 3, 3 },
	{ 2, 4, 2, 4 },
};

/* '{' */
static struct bl_brick bl_letter_123[] =
{
	{ 1, 1, 1, 5 },
	{ 2, 0, 4, 0 },
	{ 2, 6, 4, 6 },
	{ 0, 3, 1, 3 },
};

/* '|' */
static struct bl_brick bl_letter_124[] =
{
	{ 0, 0, 0, 2 },
	{ 0, 4, 0, 6 },
};

/* '}' */
static struct bl_brick bl_letter_125[] =
{
	{ 3, 1, 3, 5 },
	{ 0, 0, 2, 0 },
	{ 0, 6, 2, 6 },
	{ 4, 3, 4, 3 },
};

/* '~' */
static struct bl_brick bl_letter_126[] =
{
	{ 1, 0, 2, 0 },
	{ 4, 2, 5, 2 },
	{ 0, 1, 0, 1 },
	{ 3, 1, 3, 1 },
	{ 6, 1, 6, 1 },
};


/* big array */

static struct bl_letter letterdata[] =
{
	{ 2, 0, 0 },
	{ 3, 6, bl_letter_33 },
	{ 7, 6, bl_letter_34 },
	{ 7, 4, bl_letter_35 },
	{ 7, 8, bl_letter_36 },
	{ 7, 15, bl_letter_37 },
	{ 7, 11, bl_letter_38 },
	{ 3, 4, bl_letter_39 },
	{ 4, 5, bl_letter_40 },
	{ 4, 5, bl_letter_41 },
	{ 7, 7, bl_letter_42 },
	{ 5, 2, bl_letter_43 },
	{ 3, 4, bl_letter_44 },
	{ 5, 1, bl_letter_45 },
	{ 3, 3, bl_letter_46 },
	{ 7, 7, bl_letter_47 },
	{ 7, 8, bl_letter_48 },
	{ 6, 4, bl_letter_49 },
	{ 7, 6, bl_letter_50 },
	{ 7, 7, bl_letter_51 },
	{ 7, 3, bl_letter_52 },
	{ 7, 6, bl_letter_53 },
	{ 7, 6, bl_letter_54 },
	{ 7, 6, bl_letter_55 },
	{ 7, 7, bl_letter_56 },
	{ 7, 6, bl_letter_57 },
	{ 3, 4, bl_letter_58 },
	{ 3, 6, bl_letter_59 },
	{ 4, 7, bl_letter_60 },
	{ 5, 2, bl_letter_61 },
	{ 4, 7, bl_letter_62 },
	{ 7, 6, bl_letter_63 },
	{ 7, 7, bl_letter_64 },
	{ 7, 8, bl_letter_65 },
	{ 7, 6, bl_letter_66 },
	{ 7, 5, bl_letter_67 },
	{ 7, 4, bl_letter_68 },
	{ 7, 4, bl_letter_69 },
	{ 7, 3, bl_letter_70 },
	{ 7, 6, bl_letter_71 },
	{ 7, 3, bl_letter_72 },
	{ 3, 3, bl_letter_73 },
	{ 7, 3, bl_letter_74 },
	{ 6, 8, bl_letter_75 },
	{ 7, 2, bl_letter_76 },
	{ 7, 7, bl_letter_77 },
	{ 7, 7, bl_letter_78 },
	{ 7, 4, bl_letter_79 },
	{ 7, 4, bl_letter_80 },
	{ 7, 7, bl_letter_81 },
	{ 7, 7, bl_letter_82 },
	{ 7, 7, bl_letter_83 },
	{ 7, 2, bl_letter_84 },
	{ 7, 3, bl_letter_85 },
	{ 7, 7, bl_letter_86 },
	{ 7, 5, bl_letter_87 },
	{ 7, 13, bl_letter_88 },
	{ 7, 7, bl_letter_89 },
	{ 7, 7, bl_letter_90 },
	{ 5, 3, bl_letter_91 },
	{ 7, 7, bl_letter_92 },
	{ 5, 3, bl_letter_93 },
	{ 5, 5, bl_letter_94 },
	{ 7, 1, bl_letter_95 },
	{ 3, 4, bl_letter_96 },
	{ 6, 6, bl_letter_97 },
	{ 6, 6, bl_letter_98 },
	{ 6, 5, bl_letter_99 },
	{ 6, 4, bl_letter_100 },
	{ 6, 4, bl_letter_101 },
	{ 6, 3, bl_letter_102 },
	{ 6, 6, bl_letter_103 },
	{ 6, 3, bl_letter_104 },
	{ 1, 1, bl_letter_105 },
	{ 6, 3, bl_letter_106 },
	{ 6, 7, bl_letter_107 },
	{ 6, 2, bl_letter_108 },
	{ 6, 5, bl_letter_109 },
	{ 6, 6, bl_letter_110 },
	{ 6, 4, bl_letter_111 },
	{ 6, 4, bl_letter_112 },
	{ 6, 7, bl_letter_113 },
	{ 6, 6, bl_letter_114 },
	{ 6, 6, bl_letter_115 },
	{ 5, 2, bl_letter_116 },
	{ 6, 3, bl_letter_117 },
	{ 6, 5, bl_letter_118 },
	{ 6, 5, bl_letter_119 },
	{ 6, 10, bl_letter_120 },
	{ 5, 5, bl_letter_121 },
	{ 6, 6, bl_letter_122 },
	{ 5, 4, bl_letter_123 },
	{ 1, 2, bl_letter_124 },
	{ 5, 4, bl_letter_125 },
	{ 7, 5, bl_letter_126 },
};


/* done */

