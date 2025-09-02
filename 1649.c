#define MOD 1000000007
#define MAX_VAL 100000


int bit[MAX_VAL + 2];

void update(int index, int val) {
    while (index <= MAX_VAL) {
        bit[index] += val;
        index += index & -index;
    }
}

int query(int index) {
    int res = 0;
    while (index > 0) {
        res += bit[index];
        index -= index & -index;
    }
    return res;
}

int createSortedArray(int* instructions, int instructionsSize){
    for(int i=0;i<=MAX_VAL;i++) bit[i]=0;
    long long cost = 0;
    
    for(int i=0;i<instructionsSize;i++){
        int x = instructions[i];
        int less = query(x - 1);          
        int greater = i - query(x);       
        cost = (cost + (less < greater ? less : greater)) % MOD;
        update(x, 1);
    }
    
    return (int)cost;
}
