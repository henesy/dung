#include <u.h>
#include <libc.h>
#include <9curses.h>
#include "dungeon_generator.h"
#include "binheap.h"

/* compare two ints used as costs ;; 0 if same, <0 if higher than key; >0 if lower than key */
int compare_int(const void *key, const void *with) {
	//printf("%d\n", *(const int *) key);
	return (const int) ((*(Tile_Node *) key).cost - (*(Tile_Node *) with).cost);
}

/* returns the hardness cost of an int hardness */
int h_calc(int h) {
	int hc = 0;

	if(h >= 0 && h < 85) {
		return 1;
	}
	if(h > 84 && h < 171) {
		return 2;
	}
	if(h > 170 && h < 255) {
		return 3;
	}

	return hc;
}

/* djikstra's take 2; with tunnelling */
void
map_dungeon_t(Dungeon * dungeon)
{
	int i, j;
	binheap_t h;
	Tile_Node **tiles;
	tiles = calloc(dungeon->h, sizeof(Tile_Node*));
	for(i = 0; i < dungeon->h; i++)
		tiles[i] = calloc(dungeon->w, sizeof(Tile_Node));

	binheap_init(&h, compare_int, nil);

	/* starts from top left */
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	/* set all indices and insert the default values */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX;
			tiles[i][j].v = FALSE;
		}
	}

	/* set the player's cost as 0: */
	int px = dungeon->ss[dungeon->pc].p.x;
	int py = dungeon->ss[dungeon->pc].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);

	/* primary cost calculation logic */

	binheap_node_t	*p;

	while((p = binheap_remove_min(&h))) {
		int hx = ((Tile_Node *) p)->x;
		int hy = ((Tile_Node *) p)->y;
		int tc = ((Tile_Node *) p)->cost;

		for(i = 0; i < 8; i++) {
			int x = hx + xs[i];
			int y = hy + ys[i];
			if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
				int hard = dungeon->d[y][x].h;
				if(hard < 255) {
						int trial_cost = tc + h_calc(hard);
						if((tiles[y][x].cost > trial_cost && tiles[y][x].v == TRUE) || tiles[y][x].v == FALSE) {
							tiles[y][x].cost = tc + h_calc(hard);
							tiles[y][x].v = TRUE;

							binheap_insert(&h, (void *) &tiles[y][x]);
						}
				}
			}
		}
	}

	/* copy the heatmap to the dungeon */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->cst[i][j] = tiles[i][j].cost;
		}
	}
	
	// Free
	for(i = 0; i < dungeon->h; i++)
		free(tiles[i]);
	free(tiles);

	/* clean up the heap */
	binheap_delete(&h);
}

/* djikstra's take 2 */
void map_dungeon_nont(Dungeon * dungeon) {
	int i, j;
	binheap_t h;
	Tile_Node **tiles;
	tiles = calloc(dungeon->h, sizeof(Tile_Node*));
	for(i = 0; i < dungeon->h; i++)
		tiles[i] = calloc(dungeon->w, sizeof(Tile_Node));

	binheap_init(&h, compare_int, nil);

	/* starts from top left */
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	/* set all indices and insert the default values */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX;
			tiles[i][j].v = FALSE;
		}
	}

	/* set the player's cost as 0: */
	int px = dungeon->ss[dungeon->pc].p.x;
	int py = dungeon->ss[dungeon->pc].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);

	/* primary cost calculation logic */

	binheap_node_t	*p;

	while((p = binheap_remove_min(&h))) {
		int hx = ((Tile_Node *) p)->x;
		int hy = ((Tile_Node *) p)->y;
		int tc = ((Tile_Node *) p)->cost;

		int i;
		for(i = 0; i < 8; i++) {
			int x = hx + xs[i];
			int y = hy + ys[i];
			if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
				int hard = dungeon->d[y][x].h;
				if(hard == 0) {
						int trial_cost = tc + h_calc(hard);
						if((tiles[y][x].cost > trial_cost && tiles[y][x].v == TRUE) || tiles[y][x].v == FALSE) {
							tiles[y][x].cost = tc + h_calc(hard);
							tiles[y][x].v = TRUE;

							binheap_insert(&h, (void *) &tiles[y][x]);
						}
				}
			}
		}

	}

	/* copy the heatmap to the dungeon */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->csnt[i][j] = tiles[i][j].cost;
		}
	}

	// Free
	for(i = 0; i < dungeon->h; i++)
		free(tiles[i]);
	free(tiles);

	/* clean up the heap */
	binheap_delete(&h);
}

/* reads from a dungeon file */
void read_dungeon(Dungeon * dungeon, char * path) {
	int fd;
	fd = open(path, OREAD);
	if(fd < 0) {
		fprint(2, "FILE ERROR: Could not open dungeon fd at %s! read_dungeon()\n", path);
        exits("Dungeon save file open error in read_dungeon.");
	}

	/* read the fd-type marker */
	seek(fd, 0, 0);
	char marker[6];
	read(fd, marker, 6);

	/* read the fd version marker */
	seek(fd, 6, 0);
	u32int fd_version;
	u32int fd_version_be;
	read(fd, &fd_version_be, sizeof(u32int));
	// This might break file stuffs
	//fd_version = be32toh(fd_version_be);
	fd_version = fd_version_be;
	dungeon->v = fd_version;

	/* read the size of fd */
	seek(fd, 10, 0);
	u32int size;
	u32int size_be;
	read(fd, &size_be, sizeof(u32int));
	// This might break file size stuffs
	// size = be32toh(size_be);
	size = size_be;
	dungeon->s = size;

	/* read the hardness values in */
	seek(fd, 14, 0);
	int i;
	int j;
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			int h;
			s8int h_8;
			read(fd, &h_8, sizeof(s8int));
			h = (int) h_8;
			dungeon->d[i][j].h = h;
		}
	}

	/* read in rooms in dungeon */
	seek(fd, 1694, 0);
	/* might want to make this just counted in 4's by the loop below, but w/e, math, amirite? */
	int room_i = 0;
	int room_count = (size - 1693) / 4;
	dungeon->nr = room_count;
	dungeon->r = calloc(room_count, sizeof(Room));
	/* could probably be replaced with a getpos() call for complete-ness */
	int pos;
	for(pos = 1694; pos < size; pos += 4) {
		int x_8;
		int w_8;
		int y_8;
		int h_8;
		read(fd, &x_8, sizeof(s8int));
		read(fd,&w_8, sizeof(s8int));
		read(fd, &y_8, sizeof(s8int));
		read(fd, &h_8, sizeof(s8int));

		dungeon->r[room_i].tl.x = (s8int) x_8;
		dungeon->r[room_i].w = (s8int) w_8;
		dungeon->r[room_i].tl.y = (s8int) y_8;
		dungeon->r[room_i].h = (s8int) h_8;
		dungeon->r[room_i].br.x = ((s8int) x_8) + dungeon->r[room_i].w-1;
		dungeon->r[room_i].br.y = ((s8int) y_8) + dungeon->r[room_i].h-1;



		room_i++;
	}


	/* populate the rooms and corridors if not in rooms */
	/* add rooms to the dungeon buffer */
	int h;
	for(h = 0; h < dungeon->nr; h++) {
		for(i = dungeon->r[h].tl.y; i < dungeon->r[h].br.y+1; i++) {
			for(j = dungeon->r[h].tl.x; j < dungeon->r[h].br.x+1; j++) {
				dungeon->d[i][j].c = '.';
			}
		}
	}

	/* add corridors to the dungeon buffer */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			if(dungeon->d[i][j].c != '.' && dungeon->d[i][j].h == 0) {
				dungeon->d[i][j].c = '#';
				dungeon->d[i][j].p = 1;
			}
		}
	}


	close(fd);
}

/* writes the dungeon file to ~/.rlg327/dungeon */
void write_dungeon(Dungeon * dungeon, char * path) {
	int fd;

	/* folder creation logic */
	char * env_home = getenv("HOME");
	char * fdir_path;
	fdir_path = calloc(strlen(env_home) + 9, sizeof(char));
	strcpy(fdir_path, env_home);
	strcat(fdir_path, "/.rlg327");
	create(fdir_path, ORDWR, 0777);
	/* mkdir will return -1 when it fails, but it will fail if the fd exists so it doesn't especially matter to catch it as no output would be provided */


	fd = open(path, OWRITE);
	if(fd < 0) {
		fprint(2, "FILE ERROR: Could not open dungeon fd at %s! write_dungeon()\n", path);
        exits("File error on open of dungeon save file in write.");
	}

	/* write the fd-type marker */
	seek(fd, 0, 0);
	char marker[7];
	strcpy(marker, "RLG327");
	write(fd, marker, sizeof(char)*6);

	/* write the fd version marker */
	seek(fd, 6, 0);
	u32int fd_version = 0;
	// Might break file stuff
	//u32int fd_version_be = htobe32(fd_version);
	u32int fd_version_be = fd_version;
	write(fd, &fd_version_be, sizeof(u32int));

	/* write the size of the fd ;; unsure how to properly calculate */
	seek(fd, 10, 0);
 	u32int size = 1693 + (4 * dungeon->nr);
 	// Might break file stuff
	//u32int size_be = htobe32(size);
	u32int size_be = size;
	write(fd, &size_be, sizeof(u32int));

	/* row-major dungeon matrix */
	seek(fd, 14, 0);
	int pos = 14;
	int i;
	int j;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			seek(fd, pos, 0);
			s8int h;
			h = (s8int)(dungeon->d[i][j].h);
			write(fd, &h, sizeof(s8int));
			pos++;
		}
	}

	/* room positions ;; 4 bytes per room */
	seek(fd, 1694, 0);
	for(i = 0; i < dungeon->nr; i++) {
		s8int x = (s8int) dungeon->r[i].tl.x;
		s8int w = (s8int) dungeon->r[i].w;
		s8int y = (s8int) dungeon->r[i].tl.y;
		s8int h = (s8int) dungeon->r[i].h;

		write(fd, &x, sizeof(s8int));
		write(fd, &w, sizeof(s8int));
		write(fd, &y, sizeof(s8int));
		write(fd, &h, sizeof(s8int));
	}

	free(fdir_path);
	close(fd);
}

/* parses commandline arguments */
void test_args(int argc, char ** argv, int this, int * s, int * l, int *p, int *cp, int *nm, int *nnc) {
		if(strcmp(argv[this], "--save") == 0) {
			*s = TRUE;
		} else if(strcmp(argv[this], "--load") == 0) {
			*l = TRUE;
		} else if(strcmp(argv[this], "-f") == 0) {
			*p = TRUE;
			*cp = this+1;
			if(this+1 > argc-1) {
				print("Invalid filename argument!\n");
				*p = FALSE;
			}
		} else if(strcmp(argv[this], "--nummon") == 0) {
			*nm = atoi(argv[this+1]);
		} else if(strcmp(argv[this], "--no-ncurses") == 0) {
			*nnc = TRUE;
		}
}

/* monster list view */
void monster_list(Dungeon * dungeon) {
	clear();
	
	/* monster view array and population */
	char **mons;
	int i, j;
	s32int k;
	
	mons = calloc(dungeon->ns-1, sizeof(char*));
	for(i = 0; i < dungeon->ns-1; i++)
		mons[i] = calloc(30, sizeof(char));

	for(i = 1; i < dungeon->ns; i++) {
		char ns[6];
		char ew[5];
		
		int hd = dungeon->ss[0].p.y - dungeon->ss[i].p.y;
		int wd = dungeon->ss[0].p.x - dungeon->ss[i].p.x;
		
		if(hd > 0)
			strcpy(ns, "north");
		else
			strcpy(ns, "south");
		
		if(wd > 0)
			strcpy(ew, "west");
		else
			strcpy(ew, "east");
		
		sprint(mons[i-1], "%c, %2d %s and %2d %s", dungeon->ss[i].c, abs(hd), ns, abs(wd), ew);
	}
	
	/* secondary window */
	WINDOW *w;
	w = newwin(24, 80, 0, 0);
	Bool scroll = FALSE;
	int top = 0;
	int bot;
	if(24 < dungeon->ns -1) {
		scroll = TRUE;
		bot = 23;
	} else {
		bot = dungeon->ns -2;
	}
	
	for(;;) {
		/* put the monster view to the screen */
		for(i = top, j = 0; i < dungeon->ns -1 && i <= bot && j < 24; i++, j++) {
			mvprintw(j, 0, mons[i]);
		}
		
		/* handle user interaction */
		MLV: ;
		
		k = getch();

		switch(k) {
			case KEY_UP:
				/* scroll up */
				if(scroll == FALSE)
					goto MLV;
					
				if(top-1 >= 0) {
					top--;
					bot--;
				}
				clear();
				
				break;
			case KEY_DOWN:
				/* scroll down */
				if(scroll == FALSE)
					goto MLV;
				
				if(bot+1 < dungeon->ns-1) {
					bot++;
					top++;
				}
				clear();
				
				break;
			case 27:
				/* ESC */
				return;
				break;
			default:
				goto MLV;
		}
		
		wrefresh(w);
	}
	
	//delwin(w);
	//print_dungeon(dungeon, 0, 0);
}

/* processes pc movements ;; validity checking is in monsters.c's gen_move_sprite() */
void parse_pc(Dungeon * dungeon, Bool * run, Bool * regen) {
	GCH: ;
	s32int k;
	k = getch();
	if(k == 'Q') {
		*run = FALSE;
		return;
	}

	switch(k) {
		case 'h':
			H: ;
			dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x - 1;
			break;
		case '4':
			goto H;
		case 'l':
			L: ;
			dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x + 1;
			break;
		case '6':
			goto L;
		case 'k':
			K: ;
			dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y - 1;
			break;
		case '8':
			goto K;
		case 'j':
			J: ;
			dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y + 1;
			break;
		case '2':
			goto J;
		case 'y':
			Y: ;
			dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y - 1;
			dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x - 1;
			break;
		case '7':
			goto Y;
		case 'u':
			U: ;
			dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y - 1;
			dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x + 1;
			break;
		case '9':
			goto U;
		case 'n':
			N: ;
			dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y + 1;
			dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x + 1;
			break;
		case '3':
			goto N;
		case 'b':
			B: ;
			dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y + 1;
			dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x - 1;
			break;
		case '1':
			goto B;
		case '<':
			/* stair up */
			if(dungeon->ss[0].p.x == dungeon->su.x && dungeon->ss[0].p.y == dungeon->su.y)
				*regen = TRUE;
			break;
		case '>':
			/* stair down */
			if(dungeon->ss[0].p.x == dungeon->sd.x && dungeon->ss[0].p.y == dungeon->sd.y)
				*regen = TRUE;
			break;
		case '5':
			break;
		case ' ':
			break;
		case 'm':
			monster_list(dungeon);
			print_dungeon(dungeon, 0, 0);
			goto GCH;
		default:
			goto GCH;
	}

    /* movement validity check */
	if(dungeon->d[dungeon->ss[dungeon->pc].to.y][dungeon->ss[dungeon->pc].to.x].h > 0) {
		dungeon->ss[dungeon->pc].to.x = dungeon->ss[dungeon->pc].p.x;
		dungeon->ss[dungeon->pc].to.y = dungeon->ss[dungeon->pc].p.y;
	} else {
		dungeon->ss[dungeon->pc].p.x = dungeon->ss[dungeon->pc].to.x;
		dungeon->ss[dungeon->pc].p.y = dungeon->ss[dungeon->pc].to.y;
	}
	dungeon->ss[0].t += (100 / dungeon->ss[0].s.s);

    /* check for killing an NPC */
    int sn = 0;
    int i;
	for(i = 1; i < dungeon->ns; i++) {
		if(i != sn) {
			if((dungeon->ss[i].to.x == dungeon->ss[sn].to.x) && (dungeon->ss[i].to.y == dungeon->ss[sn].to.y) && dungeon->ss[sn].a == TRUE)
				dungeon->ss[i].a = FALSE;
        }
    }
}


/* Basic procedural dungeon generator */
int main(int argc, char * argv[]) {
	/*** process commandline arguments ***/
	int max_args = 8;
	int saving = FALSE;
	int loading = FALSE;
	int pathing = FALSE;
	int nnc = FALSE;
	int num_mon = 1;
	int custom_path = 0;
	if(argc > 2 && argc <= max_args) {
		/* both --save and --load */
		int i;
		for(i = 1; i < argc; i++) {
			test_args(argc, argv, i, &saving, &loading, &pathing, &custom_path, &num_mon, &nnc);
		}
	} else if(argc == 2) {
		/* one arg */
		test_args(argc, argv, 1, &saving, &loading, &pathing, &custom_path, &num_mon, &nnc);
	} else if(argc > max_args) {
		/* more than 2 commandline arguments, argv[0] is gratuitous */
		print("Too many arguments!\n");
	} else {
		/* other; most likely 0 */
	}
	/*** end processing commandline arguments ***/


	/* init the dungeon with default dungeon size and a max of 12 rooms */
	srand(time(nil));

	/* create 2 char pointers so as not to pollute the original HOME variable */
	char * env_path = getenv("HOME");
	/* char * path = calloc(strlen(env_path) + 17, sizeof(char)); */
	char * path = calloc(strlen(env_path) + 50, sizeof(char));
	strcpy(path, env_path);
	strcat(path, "/.rlg327");
	if(pathing == TRUE) {
		strcat(path, "/");
		strcat(path, argv[custom_path]);
	} else {
		strcat(path, "/dungeon");
	}

	/* persistent player character */
	Bool regen = FALSE;
	Sprite p_pc;
	
	/*** dungeon generation starts here ***/
	DUNGEN: ;

	Dungeon dungeon = init_dungeon(21, 80, 12);

	if(loading == FALSE) {
		gen_dungeon(&dungeon);
		gen_corridors(&dungeon);
	} else {
		read_dungeon(&dungeon, path);
	}
	/*** dungeon is fully initiated ***/
	Sprite pc = gen_sprite(&dungeon, '@', -1, -1, 1);
	add_sprite(&dungeon, pc);

	int i;
	for(i = 0; i < num_mon; i++) {
		Sprite m = gen_sprite(&dungeon,'m' , -1, -1, 1);
		m.sn = i;
		add_sprite(&dungeon, m);
	}

	map_dungeon_nont(&dungeon);
	map_dungeon_t(&dungeon);
	/*** dungeon is fully generated ***/

    //binheap_t h;
	//binheap_init(&h, compare_move, nil);

	/* main loop */
	//Event nexts[dungeon.ns]
	
	if(regen == TRUE) {
		int px = dungeon.ss[0].p.x;
		int py = dungeon.ss[0].p.y;
		dungeon.ss[0] = p_pc;
		dungeon.ss[0].p.x = px;
		dungeon.ss[0].p.y = py;
		dungeon.ss[0].to.x = px;
		dungeon.ss[0].to.y = py;
	}
	

	for(i = 1; i < dungeon.ns; i++) {
		gen_move_sprite(&dungeon, i);
		//nexts[i] = next;
	}
	
	if(regen == TRUE)
		goto PNC;
	
	// Init ncurses
	initscr();
	raw();
	noecho();
	curs_set(0);
	set_escdelay(25);


	PNC: ;
	regen = FALSE;

	print_dungeon(&dungeon, 0, 0);
	Bool first = TRUE;
	Bool run = TRUE;
	while(run == TRUE) {
		//s32int key;
		//key = getch();

		int l = 0;
		for(i = 0; i < dungeon.ns; i++) {
			if(dungeon.ss[i].t < dungeon.ss[l].t) {
				l = i;
			}
		}


		if(l == dungeon.pc || first == TRUE) {
			parse_pc(&dungeon, &run, &regen);
			if(regen == TRUE) {
				p_pc = dungeon.ss[0];
				goto DUNFREE;
			}
			
			//gen_move_sprite(&dungeon, l);
			map_dungeon_nont(&dungeon);
			map_dungeon_t(&dungeon);
			
			int sn = 0;
			for(i = 1; i < dungeon.ns; i++) {
				if(i != sn) {
					if((dungeon.ss[i].p.x == dungeon.ss[sn].p.x) && (dungeon.ss[i].p.y == dungeon.ss[sn].p.y) && dungeon.ss[sn].a == TRUE)
						dungeon.ss[i].a = FALSE;
				}
			}
			
			print_dungeon(&dungeon, 0, 0);
		} else {
			parse_move(&dungeon, l);
			gen_move_sprite(&dungeon, l);
		}


		//print_dungeon(&dungeon, 1, 0); /* prints non-tunneling dijkstra's */
		//print_dungeon(&dungeon, 0, 1); /* prints tunneling dijkstra's */

		//clear();
		refresh();
		/** --- game over sequence checking --- **/
		/* note: this will stop the game before the new world gets drawn since the monster will move to the player and thus kill him */
		if(dungeon.go == TRUE || dungeon.ss[dungeon.pc].a == FALSE)
			break;

		Bool any = check_any_monsters(&dungeon);
		if(any == FALSE) {
			//printf("You win!\n");
			goto END;
		}
		first = FALSE;
	}
	print_dungeon(&dungeon, 0, 0);
	//printf("Game Over!\n");

	/*** tear down sequence ***/
	//binheap_delete(&h);
	END: ;
	delwin(stdscr);
	endwin();

	if(saving == TRUE) {
		write_dungeon(&dungeon, path);
	}

	DUNFREE: ;
	/* free our arrays */
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.d[i]);
	}
	free(dungeon.d);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.p[i]);
	}
	free(dungeon.p);
	free(dungeon.r);
	free(dungeon.ss);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.csnt[i]);
	}
	free(dungeon.csnt);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.cst[i]);
	}
	free(dungeon.cst);
	
	if(regen == TRUE)
		goto DUNGEN;
	
	free(path);
	return 0;
}
