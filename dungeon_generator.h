#ifndef dungeon_h
#define dungeon_h

/* set up Booleans */
#define	TRUE	1
#define	FALSE	0
typedef int Bool;

#define INT_MAX sizeof(int)

/* custom structures */
typedef struct {
	int		h;	/* hardness */
	char	c;	/* visual character */
	int		p;	/* mark 1 if path, 0 if not a path (corridors) */
} Tile;

typedef struct {
	int	x; /* x coordinate */
	int	y; /* y coordinate */
} Position;

/* maybe make these pointers? */
typedef struct {
	int prev; /* previous room in the path (using Room.id) */
	int next; /* room the path leads to (using Room.id) */
} Path;

typedef struct {
    Bool    in; /* intelligence */
    Bool    te; /* telepathy */
    Bool    tu; /* tunneling ability */
    Bool    eb; /* erratic behaviour */
    int     s;  /* speed ;; pc has 10 ; 5-20 */
} Stats;

typedef struct {
	Position	p;	/* position of the sprite in the dungeon */
	char		c;	/* character to print for the sprite */
    Stats       s;	/* stats for a sprite */
	int			t;	/* turn count */
	Position	to;	/* to move to */
	int			sn;	/* sprite number */
	Position	pc;	/* last known location of the PC */
	Bool		a;	/* alive T/F */
} Sprite;

typedef struct {
	Position	tl;		/* top left coordinate of the room, used as the core point of reference */
	Position	br;		/* bottom right coordinate of the room as per above */
	int			w;		/* width */
	int			h;		/* height */
	int			id;		/* room ID, potentially useful for organization */
	int			p;		/* mark 1 if processed; 0 if not processed (corridors) */
	Position	ctr;	/* "center" point; very rough, might need improved */
	int			c;		/* if connected or not; TRUE/FALSE switch */
} Room;

typedef struct {
	Tile 	**	d;		/* dungeon buffer */
	Tile 	**	p;		/* print buffer */
	int			h;		/* height */
	int			w;		/* width */
	int			nr; 	/* number of rooms */
	int			mr;		/* max rooms */
	Room 	*	r;		/* rooms buffer */
	int			v;		/* file version */
	int			s;		/* file size */
	Sprite	*	ss;		/* sprite array */
	int			ns;		/* number of sprites */
	int			ms;		/* max number of sprites */
	int		**	csnt;
	int		**	cst;	/* costs for djikstra's map */
	int			pc;		/* location of PC in SpriteS array (.ss) */
    int         t;      /* turn number */
	Bool		go;		/* game over T/F */
	Position	su;		/* location of the dungeon's up staircase */
	Position	sd;		/* location of the dungeon's down staircase */
} Dungeon;

typedef struct {
	int x;
	int y;
	int cost;
	int v;
} Tile_Node;

typedef struct {
	int			sn;	/* sprite number */
	int 		speed;	/* speed of the Sprite */
	int			turn;	/* turn counter */
	Position	to;		/* where we move to */
} Event;


/*** Function prototypes ***/
/* monsters.c */
void add_sprite(Dungeon * dungeon, Sprite s);
Sprite gen_sprite(Dungeon * dungeon, char c, int x, int y, int r);
void gen_move_sprite(Dungeon * dungeon, int sn);
void parse_move(Dungeon * dungeon, int sn);
Bool check_any_monsters(Dungeon * dungeon);
Bool test_loc(Dungeon * dungeon, int x, int y, Sprite *s);
void with_pc(Dungeon * dungeon, Sprite * s, Bool *in);

/* world.c */
void gen_corridors(Dungeon * dungeon);
void gen_dungeon(Dungeon * dungeon);
Dungeon init_dungeon(int h, int w, int mr);

/* printing.c */
void print_hardness(Dungeon * dungeon);
void print_t_heatmap(Dungeon * dungeon);
void print_nont_heatmap(Dungeon * dungeon);
void print_dungeon(Dungeon * dungeon, int nt, int t);
void print_dungeon_nnc(Dungeon * dungeon, int nt, int t);

/* main.c */
void map_dungeon_t(Dungeon * dungeon);

#endif
