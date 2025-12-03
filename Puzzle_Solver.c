#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 4
#define NxN (N*N)
#define TRUE 1
#define FALSE 0

struct node {
	int tiles[N][N];
	int f, g, h;
	short zero_row, zero_column;	/* location (row and colum) of blank tile 0 */
	struct node *next;
	struct node *parent;			/* used to trace back the solution */
};

int goal_rows[NxN];
int goal_columns[NxN];
struct node *start,*goal;
struct node *open = NULL, *closed = NULL;
struct node *succ_nodes[4];

void print_a_node(struct node *pnode) {
	int i,j;
	for (i=0;i<N;i++) {
		for (j=0;j<N;j++) 
			printf("%2d ", pnode->tiles[i][j]);
		printf("\n");
	}
	printf("\n");
}

struct node *initialize(char **argv){
	int i,j,k,index, tile;
	struct node *pnode;
	
	pnode=(struct node *) malloc(sizeof(struct node));
	index = 1;
	for (j=0;j<N;j++)
		for (k=0;k<N;k++) {
			tile=atoi(argv[index++]);
			pnode->tiles[j][k]=tile;
			if(tile==0) {
				pnode->zero_row=j;
				pnode->zero_column=k;
			}
		}
	pnode->f=0;
	pnode->g=0;
	pnode->h=0;
	pnode->next=NULL;
	pnode->parent=NULL;
	start=pnode;
	printf("initial state\n");
	print_a_node(start);

	pnode=(struct node *) malloc(sizeof(struct node));
	goal_rows[0]=3;
	goal_columns[0]=3;

	for(index=1; index<NxN; index++){
		j=(index-1)/N;
		k=(index-1)%N;
		goal_rows[index]=j;
		goal_columns[index]=k;
		pnode->tiles[j][k]=index;
	}
	pnode->tiles[N-1][N-1]=0;	      /* empty tile=0 */
	pnode->f=0;
	pnode->g=0;
	pnode->h=0;
	pnode->next=NULL;
	goal=pnode; 
	printf("goal state\n");
	print_a_node(goal);

	return start;
}

/* merge the remaining nodes pointed by succ_nodes[] into open list. 
 * Insert nodes based on their f values --- keep f values sorted. */
void merge_to_open() { 
	//Make a for loop that keeps track of every succ_nodes' pointer. If it points to null, ignore it, else add to open. If f is smaller than previous open node, insert it before that nodes.
	struct node *curr_node, *prev_node, *pnode;
	for(int i=0; i<4; i++){
		pnode = succ_nodes[i];
		if(pnode != NULL){
			if(open == NULL || pnode->f < open->f){
				pnode->next = open;
				open = pnode;
			}else{
				prev_node=open;
				curr_node = open->next;
				int insert = 0;
				while(curr_node != NULL){
					if(pnode->f <= curr_node->f){
					       if(pnode->h <= curr_node->h)break;
					}
					curr_node = curr_node->next;
				}
				pnode->next = curr_node;
				prev_node->next = pnode;
			}

		}	
	}	
}

/*swap two tiles in a node*/
void swap(int row1,int column1,int row2,int column2, struct node * pnode){
	int tile = pnode->tiles[row1][column1];
	pnode->tiles[row1][column1] = pnode->tiles[row2][column2];
	pnode->tiles[row2][column2] = tile;
}

/*update the f,g,h function values for a node */
void update_fgh(int i) {
	struct node *pnode = succ_nodes[i];
	if(pnode == NULL) return;
	pnode->g = pnode->parent->g+1;
	for(int j = 0; j < 4; j++){
		for(int k = 0; k < 4; k++){
			int tile = pnode->tiles[j][k];
			if(tile != 0){
				pnode->h += (abs(goal_rows[tile] - j) + abs(goal_columns[tile] - k));
			}
		}
	}
	pnode->f = pnode->g + pnode->h;
}

/* 0 goes down by a row */
void move_down(struct node * pnode){
	if(pnode->zero_row + 1 < N){
		swap(pnode->zero_row, pnode->zero_column, pnode->zero_row + 1, pnode->zero_column, pnode);
       	pnode->zero_row++;
	}else{
		pnode = NULL;
	}	
}

/* 0 goes right by a column */
void move_right(struct node * pnode){
	if(pnode->zero_column + 1 < N){
		swap(pnode->zero_row, pnode->zero_column, pnode->zero_row, pnode->zero_column + 1, pnode);
		pnode->zero_column++;
	}else{
		pnode = NULL;
	}
}

/* 0 goes up by a row */
void move_up(struct node * pnode){
	if(pnode->zero_row - 1 > -1){
		swap(pnode->zero_row, pnode->zero_column, pnode->zero_row - 1, pnode->zero_column, pnode);
		pnode->zero_row--;
	}else{
		pnode=NULL;
	}
}

/* 0 goes left by a column */
void move_left(struct node * pnode){
	if(pnode->zero_column - 1 > -1){
		swap(pnode->zero_row, pnode->zero_column, pnode->zero_row, pnode->zero_column - 1, pnode);
		pnode->zero_column--;
	}else{
		pnode = NULL;
	}
}

/* expand a node, get its children nodes, and organize the children nodes using
 * array succ_nodes.
 */
void expand(struct node *selected) {
	for(int i = 0; i<4; i++){
		succ_nodes[i] = (struct node *)malloc(sizeof(struct node));
		memcpy(succ_nodes[i]->tiles, selected->tiles, NxN*sizeof(int));
		succ_nodes[i]->zero_row = selected->zero_row;
		succ_nodes[i]->zero_column = selected->zero_column;
		succ_nodes[i]->parent = selected;
	}
	move_right(succ_nodes[0]);
	move_left(succ_nodes[1]);
	move_up(succ_nodes[2]);
	move_down(succ_nodes[3]);
}

int nodes_same(struct node *a,struct node *b) {
	int flg=FALSE;
	if (memcmp(a->tiles, b->tiles, sizeof(int)*NxN) == 0)
		flg=TRUE;
	return flg;
}

/* Filtering. Some states in the nodes pointed by succ_nodes may already be included in 
 * either open or closed list. There is no need to check these states. Release the 
 * corresponding nodes and set the corresponding pointers in succ_nodes to NULL. This is 
 * important to reduce execution time.
 * This function checks the (i)th node pointed by succ_nodes array. You must call this
 * function in a loop to check all the nodes in succ_nodes. Free the successor node and 
 * set the corresponding pointer to NULL in succ_node[] if a matching node is found in 
 * the list.
 */ 
void filter(int i, struct node *pnode_list){ 
	struct node *pnode = succ_nodes[i];
	if(pnode == NULL) return;
	struct node *list = pnode_list;
	while(list != NULL){
		if(nodes_same(list, succ_nodes[i])){
			free(pnode);
			succ_nodes[i]=NULL;
			return;
		}
		list=list->next;
	}
}

int main(int argc,char **argv) {
	int iter,cnt;
	struct node *copen, *cp, *solution_path;
	int ret, i, pathlen=0, index[N-1];

	solution_path=NULL;
	start=initialize(argv);	/* init initial and goal states */
	open=start; 

	iter=0; 
	while (open!=NULL) {	/* Termination cond with a solution is tested in expand. */
		copen=open;
		open=open->next;  /* get the first node from open to expand */

		/* DEBUG: print the node (layout) in *copen 
		 * Fix segmentation faults first. If the program cannot produce correct results,
		 * select an initial layout close to the goal layout, and debug the program with it.
		 * gdb is recommended. You may also print data on the screen. But this is not as
		 * convenient as using gdb.
		 * */

		if(nodes_same(copen,goal)){ /* goal is found */
			do{ /* trace back and add the nodes on the path to a list */
				copen->next=solution_path;
				solution_path=copen;
				copen=copen->parent;
				pathlen++;
			} while(copen!=NULL);
			printf("Path (lengh=%d):\n", pathlen); 
			copen=solution_path;
			while(solution_path!=NULL){
			       print_a_node(solution_path);
		 		solution_path=solution_path->next;
		 	}
			printf("%d\n", pathlen);		
			       /* print out the nodes on the list */
			exit(0);
		}
		expand(copen);       /* Find new successors */

		/* DEBUG: print the layouts/nodes organized by succ_nodes[] */
	/*	
		for(i=0;i<4;i++){
			printf("Success nodes\n");
			print_a_node(succ_nodes[i]);
		}
	*/	
		for(i=0;i<4;i++){
			filter(i,open);
			filter(i,closed);
			update_fgh(i);
		}

		/* DEBUG: print the layouts/modes remaining in succ_nodes[] */
		/*
		for(i=0;i<4;i++){
			if(succ_nodes[i] != NULL){
				printf("New Nodes\n");
				print_a_node(succ_nodes[i]);
			}
		}
		*/
		
		merge_to_open(); /* New open list */
		struct node *temp = open;
		/*
		while(temp != NULL){
			printf("f -> %d\n", temp->f);
			print_a_node(temp);
			temp = temp->next;
		}
		*/
		/* DEBUG: print the layouts/nodes on the open list */
		copen->next=closed;
		closed=copen;		/* New closed */
		/* print out something so that you know your 
		 * program is still making progress 
		 */
		//printf("Progress made!\n");
		//exit(0);
		iter++;
		if(iter %1000 == 0)
			printf("iter %d\n", iter);
	}
	printf("No solutions!\n");
	return 0;
} /* end of main */
