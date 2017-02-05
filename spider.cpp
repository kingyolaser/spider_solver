
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <openssl/md5.h>

/****************************************************************************/
typedef enum{
    spade, heart, diamond, club, suit_unknown=-1,
}Suit;

const char suit_char[] = {'s','h','d','c'};
enum{
    card_empty=0, card_unknown=-1,
};
class Card{
public:
    
    Suit  suit;
    int   n;
    bool  invisible;
    
    Card(): suit(suit_unknown),n(1),invisible(false){}
    Card(Suit s, int n_): suit(s), n(n_), invisible(false){}

};

/****************************************************************************/
class Katamari{
public:
    Suit   s;
    int    position;  //bottomの配列index
    int    top;
    int    bottom;
    
    Katamari():s(suit_unknown),position(0),top(0),bottom(0){}
    Katamari(const Card* card_array):s(suit_unknown),position(0),top(0),bottom(0){init(card_array);}
    void init(const Card* card_array);
    
    bool isEmpty()const{return top==0;}
};
/****************************************************************************/
#define PRIORITY_TYPE_MASK     0xFFFF0000  //上位4bit=TYPE, 下位4bit=同TYPE内優先度

#define PRIORITY_TYPE_MOVE     0x40000000
#define PRIORITY_TYPE_DRAW     0x20000000

#define PRIORITY_FOR_KATAZUKE  0x40800000

#define PRIORITY_SAMESUIT      0x40400000
#define PRIORITY_DIFFERENTSUIT 0x40200000
#define PRIORITY_KATAMARI_DOWN 0x40100000
#define PRIORITY_DRAW          0x20010000
#define PRIORITY_MIN           1
class Move{
public:
    Move():type_and_priority(PRIORITY_MIN) {}

    int from;  //-1 means draw
    int to;
    //int num;   //塊を分割して、emptyへ移動させるときの枚数。toがemptyでなければ0を入れる。
    Katamari k;
    int  type_and_priority; //数字が大きいほど優先
    bool isDraw()const{return from==-1;}
};

/****************************************************************************/
void Katamari::init(const Card* card_array)
{
    //まず長さをチェック
    int len;
    for( len=0; ; len++){
        if( card_array[len].n == 0 ){
            break;
        }
    }
    
    //emptyチェック
    if( len==0 ){
        top = bottom = 0;
        return;
    }
    
    //emptyでない。塊を解析。
    top = card_array[len-1].n;
    s = card_array[len-1].suit;
    int y;
    for( y=len-1; y>=0; y-- ){
        if( y==len-1 ){ continue; } //１周目はなにもしない。
        if( card_array[y].invisible ){ break; }
        if( card_array[y].suit != s ){ break; }
        if( card_array[y].n != card_array[y+1].n+1 ){
            break;
        }
    }
    position = y+1;
    bottom   = card_array[position].n;
}
/****************************************************************************/
class Candidate{
public:
    Move m;
};
/****************************************************************************/
#define MAX_REMOVE 7  //１手での最大片付山数。draw時に配ったカードで同時に片づけられたケース。
class History{
public:
    Move m;
    bool frompile_fliped;
    int  remove_num;
    int  remove_x[MAX_REMOVE];
    Suit remove_suit[MAX_REMOVE];
    bool removepile_fliped[MAX_REMOVE]; //除去後にその下をめくったかどうか
    unsigned char md5sum[MD5_DIGEST_LENGTH];  //ループチェック用
    
    void init(){ frompile_fliped=false; remove_num=0; }
};
/****************************************************************************/
class Board{
public:
    enum{
        WIDTH = 10, STOCK_MAX=5, HISTORY_MAX=512,
    };

    Card  tableau[WIDTH][104];
    int   tesuu;
    Card  stock[WIDTH][5];
    int   stock_remain;
    int   kataduke;
    History  history[HISTORY_MAX];

    Board(){init();}
    void init();
    void init(int argc, const char* argv[]);  //mainに渡されるものからargv[0]を除いて渡すこと
    void print()const;

    bool isComplete()const{for(int x=0;x<WIDTH;x++)if(tableau[x][0].n!=0)return false; return true;}
    bool existEmpty()const{return getEmpty()!=-1;}
    int  getEmpty()const{for(int x=0;x<WIDTH;x++)if(tableau[x][0].n==0)return x; return -1;}
    bool canMove(Move m)const;
    void getHash(unsigned char getHash[MD5_DIGEST_LENGTH])const;
    bool isLooping()const;
    
    void search_candidate(Candidate *candidate, int *num)const;
    
    void inquire(int x, int y);
    void doMove(const Move &m);
    void doDraw(History &h);
    void check_remove(int x, History &h);
    void undo();
    void undo_draw();
};
/****************************************************************************/
int c2i(char c)
{
    c = toupper(c);
    const char table[] = " A234567890JQK";
    const char *pos = strchr( table, c);
    if( pos ){return pos-table;}
    if( c=='1' ){return 1;}
    printf("convert err c=%c\n",c);
    return -1;
}
/****************************************************************************/
Suit c2s(char c)
{
    c = tolower(c);
    const char *pos = strchr( suit_char, c);
    if( pos ){return (Suit)(pos-suit_char);}
    return suit_unknown;
}
/****************************************************************************/
void Board::init()
{
    memset(tableau, 0, sizeof(tableau));
    tesuu = 0;
    memset(stock, 0, sizeof(stock));
    stock_remain = 0;
    kataduke = 0;
}
/****************************************************************************/
void Board::init(int argc, const char* argv[])
{
    assert( argc>=11 );
    init();
    
    //場の設定
    for( int x=0; x<WIDTH; x++){
        size_t len = strlen(argv[x])/3;
        for( size_t y=0; y<len; y++ ){
            tableau[x][y].invisible = (argv[x][y*3]=='x'? true:false);
            if( argv[x][y*3+2]=='x'){
                tableau[x][y].n    = card_unknown;
                tableau[x][y].suit = suit_unknown;
            }else{
                tableau[x][y].n    = c2i(argv[x][y*3+2]);
                tableau[x][y].suit = c2s(argv[x][y*3+1]);
            }
        }
    }

    //ストックの設定
    assert(strcmp(argv[WIDTH],"-")==0);
    for( int i=argc-1; i>=WIDTH+1; i--){
        assert(strlen(argv[i])==WIDTH*2);
        for( int x=0; x<WIDTH; x++){
            stock[x][stock_remain].n    = c2i(argv[i][x*2+1]);
            stock[x][stock_remain].suit = c2s(argv[i][x*2]);
        }
        stock_remain++;
    }
}
/****************************************************************************/
void Board::print() const
{
    int count[14]={0};
    int suit_count[4]={0};
    
    printf("\n");
    for( int y=0; ;y++){
        bool exist = false;
        for( int x=0; x<WIDTH; x++){
            if( tableau[x][y].n == 0 ){
                printf("      ");
            }else if( tableau[x][y].invisible ){
                if( tableau[x][y].n == card_unknown ){
                    printf(" =xx= ");
                }else{
                    printf(" =%c%c= ",suit_char[tableau[x][y].suit],
                                     " 1234567890JQK*"[tableau[x][y].n]);
                    count[tableau[x][y].n]++;
                    suit_count[tableau[x][y].suit]++;
                }
                exist = true;
            }else{
                printf("  %c%c  ",suit_char[tableau[x][y].suit],
                                 " 1234567890JQK*"[tableau[x][y].n]);
                count[tableau[x][y].n]++;
                suit_count[tableau[x][y].suit]++;
                exist = true;
            }
        }
        printf("\n");
        if( !exist ){break;}
    }
    
    //stock表示
    printf("stock:");
    for( int i=0; i<stock_remain; i++){
        printf("\n");
        for( int x=0; x<WIDTH; x++){
            printf("%c%c ", suit_char[stock[x][i].suit],
                            " 1234567890JQK*"[stock[x][i].n]);
            count[stock[x][i].n]++;
            suit_count[stock[x][i].suit]++;
        }
    }
    printf("\n");
    
    printf("cards: %d%d%d%d%d%d%d%d%d%d%d%d%d %d %d %d %d\n",
        count[1],count[2],count[3],count[4],count[5],count[6],count[7],
        count[8],count[9],count[10],count[11],count[12],count[13],
        suit_count[0],suit_count[1],suit_count[2],suit_count[3]);
    printf("kataduke=%d",kataduke);
    printf("\ntesuu=%d\n", tesuu);

    //History
    printf("history: ");
    for( int i=0; i<tesuu; i++){
        if( history[i].m.isDraw() ){
            printf("draw:");
        }else{
            printf("%d%d:", history[i].m.from, history[i].m.to);
        }
    }
    printf("\n");
}
/****************************************************************************/
void Board::getHash(unsigned char getHash[MD5_DIGEST_LENGTH])const
{
    MD5_CTX c;
    int r;
    char buf[80];
    
    r = MD5_Init(&c);
    assert(r==1);

    //----------ハッシュ値積算
    for( int x=0; x<WIDTH; x++ ){
        for( int y=0; ; y++){

            if( tableau[x][y].n == 0 ){
                break;
            }else if( tableau[x][y].invisible ){
                if( tableau[x][y].n == card_unknown ){
                    r = MD5_Update(&c, "=xx=", strlen("=xx=")); assert(r==1);
                }else{
                    sprintf(buf, " =%c%c= ",suit_char[tableau[x][y].suit],
                                     " 1234567890JQK*"[tableau[x][y].n]);
                    r = MD5_Update(&c, buf, strlen(buf)); assert(r==1);
                }
            }else{
                sprintf(buf, "  %c%c  ",suit_char[tableau[x][y].suit],
                                 " 1234567890JQK*"[tableau[x][y].n]);
                r = MD5_Update(&c, buf, strlen(buf)); assert(r==1);
            }
        }
        r = MD5_Update(&c, "-", 1); assert(r==1);  //区切り
    }

    //stockのハッシュ積算
    r = MD5_Update(&c, &stock_remain, sizeof(stock_remain)); assert(r==1);

    //----------ハッシュ値積算　終わり

    
    r = MD5_Final(getHash, &c);  assert(r==1);
}

/****************************************************************************/
bool Board::isLooping()const
{
    if( tesuu<=1 ){
        return false; //0～1手のみではループにならない
    }

    for( int i=0; i<tesuu-2; i++){
        if( memcmp(history[tesuu-1].md5sum, history[i].md5sum, MD5_DIGEST_LENGTH)==0 ){
            //printf("loop detected.\n");
            return true;
        }
    }
    return false;
}
/****************************************************************************/
int candidate_compare(const void *a_, const void *b_)
{
    const Candidate *a = (Candidate*)a_;
    const Candidate *b = (Candidate*)b_;
    
    if( a->m.type_and_priority < b->m.type_and_priority ){
        return 1;
    }else if( a->m.type_and_priority == b->m.type_and_priority ){
        return 0;
    }else{
        return -1;
    }
}
void Board::search_candidate(Candidate *candidate, int *num)const
{
    *num = 0;

    if( isLooping() ){
        return; //ループ→選択肢なし→どんずまりとする
    }
    
    if( tesuu >=HISTORY_MAX){
        return; //手数限界：たぶん初期のほうで誤った手を指して、泥沼
    }

    //同一の塊が無駄な動きを禁則とする
    if( tesuu>=2 ){
        const History &h1 = history[tesuu-2];
        const History &h2 = history[tesuu-1];
        if( ! h1.m.isDraw() && !h2.m.isDraw()
           && h1.m.to == h2.m.from && h1.m.k.bottom==h2.m.k.bottom ){
            if( h1.frompile_fliped && h2.m.to==h1.m.from ){
                //許可する。動かして下めくって、再び戻すケース
            }else{
                return; //それ以外は禁則
            }
        }
    }
    
    //独立したMoveは、Fromが小さいものを先に行うルールとする。
    //反したMoveの場合は禁則
    if( tesuu>=2 ){
        const History &h1 = history[tesuu-2];
        const History &h2 = history[tesuu-1];
        if( ! h1.m.isDraw() && !h2.m.isDraw()
           && h1.m.from != h2.m.from && h1.m.to != h2.m.to
           && h1.m.from != h2.m.to   && h1.m.to != h2.m.from ){//互いに独立
            if( h1.m.from < h2.m.from ){
                //許可する。
            }else{
                return; //禁則
            }
        }
    }

    Katamari k[WIDTH];
    
    for( int from=0; from<WIDTH; from++){
        k[from].init(tableau[from]);
        //printf("%d-%d\n", k[from].top, k[from].bottom);
    }
    
    //kingからの山に積み重ねられるかチェック
    //塊全体でなくてよい。同一suit必須
    for( int to=0; to<WIDTH; to++){
        if( k[to].bottom==13 ){
            for( int from=0; from<WIDTH; from++){
                if( from==to ){ continue; }
                if( k[to].s != k[from].s ){continue;}
                if( k[from].bottom ==13 ){continue;}
                if( k[from].top<k[to].top && k[to].top <= k[from].bottom ){
                    //↑この条件で、塊全体移動は除外されている。
                    Katamari partial_k = k[from];
                    partial_k.bottom   = k[to].top-1;
                    partial_k.position = k[from].position + (k[from].bottom - k[to].top+1);
                    
                    candidate[*num].m.from = from;
                    candidate[*num].m.to   = to;
                    candidate[*num].m.k    = partial_k;
                    candidate[*num].m.type_and_priority = PRIORITY_FOR_KATAZUKE;
                    (*num)++;
                    //printf("from=%d, to=%d, %d-%d\n", from, to, partial_k.top, partial_k.bottom);
                    //exit(0);
                }
            }
        }
    }
    if( *num!=0 ){  //この時点で候補があったら打ち止め
        return;
    }

    //まだ候補ゼロ
    //塊全体移動のチェック。suit違っても候補。
    for( int from=0; from<WIDTH; from++){
        for( int to=0; to<WIDTH; to++){
            if( from==to ){continue;}
            if( k[from].isEmpty() || k[to].isEmpty() ){continue;}
            if( k[from].bottom +1 == k[to].top ){
                candidate[*num].m.from = from;
                candidate[*num].m.to   = to;
                candidate[*num].m.k    = k[from];
                candidate[*num].m.type_and_priority = (k[from].s==k[to].s)? PRIORITY_SAMESUIT : PRIORITY_DIFFERENTSUIT;
                (*num)++;
            }
        }
    }
    
    
    //empty spaceがある場合、塊を下す手を列挙
    //
    int x;
    if( (x=getEmpty())!=-1 ){
        for( int from=0; from<WIDTH; from++ ){
            if( k[from].position != 0 ){ //塊は下す価値がある
                candidate[*num].m.from = from;
                candidate[*num].m.to   = x;
                candidate[*num].m.k    = k[from];
                candidate[*num].m.type_and_priority = PRIORITY_KATAMARI_DOWN;
                (*num)++;
            }
        }
    }
    
    if( stock_remain>=1 ){
        if( !existEmpty() ){
            candidate[*num].m.from = -1;
            candidate[*num].m.type_and_priority = PRIORITY_DRAW;
            (*num)++;
        }else if(*num==0){
            print();
            printf("ほとんど解決だがstock消化要\n");
            printf("あとは自分でやってください。\n");
            exit(0);
        }
    }

    //sort
    qsort(candidate, *num, sizeof(Candidate), candidate_compare);

#if 1
    //この時点で、同一suitへの移動があったら打ち止め
    //優先度topが同一suit連結の場合、それ以下の優先度はカットする。
    if( *num>=1 && candidate[0].m.type_and_priority>=PRIORITY_SAMESUIT ){
        for( int i=1; i<*num; i++){
            if( candidate[i].m.type_and_priority<PRIORITY_SAMESUIT ){

                //debug
                #if 0
                print();
                for( int j=0; j<*num; j++){
                    printf("%d%d:", candidate[j].m.from, candidate[j].m.to);
                }
                printf("\n");
                printf("cut low priority move.\n");
                printf("candidate num= %d -> %d\n", *num,i);
                exit(1);
                #endif

                *num=i;
                return;
            }
        }
    }
#endif

    //printf("candidate num=%d\n", *num);
}
/****************************************************************************/
void Board::inquire(int x, int y)
{
    print();

    Card ret;
    char buf[20];
    for(;;){
        printf("input(%d,%d)> ",x,y);
        fgets(buf, sizeof(buf), stdin );
        ret.suit = c2s(buf[0]);
        ret.n    = c2i(buf[1]);
        if( ret.suit!=suit_unknown || ret.n!=card_unknown ){
            tableau[x][y] = ret;
            return;
        }
    }
}

/****************************************************************************/
void Board::doMove(const Move &m)
{
    assert(tesuu<HISTORY_MAX);
    history[tesuu].init();
    history[tesuu].m = m;

    if( ! m.isDraw() ){
        //printf("Moving: %d->%d\n", m.from, m.to);
        //toの高さ
        int to_hight;
        for( to_hight=0; ; to_hight++ ){
            if( tableau[m.to][to_hight].n==0 ){
                break;
            }
        }
        
        //一応チェック
        assert(to_hight==0 || tableau[m.to][to_hight-1].n==m.k.bottom+1);
        
        //コピー&消去
        for( int from_y=m.k.position; from_y<=m.k.position+(m.k.bottom-m.k.top) ; to_hight++,from_y++ ){
            tableau[m.to][to_hight] = tableau[m.from][from_y];
            tableau[m.from][from_y].n = 0;
        }
        
        //移動元のtopを表にする。
        if( m.k.position >=1 ){
            if( tableau[m.from][m.k.position-1].invisible ){
                if( tableau[m.from][m.k.position-1].n == card_unknown ){
                    printf("Moved %d->%d\n", m.from, m.to);
                    inquire(m.from, m.k.position-1);
                }
                tableau[m.from][m.k.position-1].invisible = false;
                history[tesuu].frompile_fliped = true;
            }
        }
    
        //除去チェック
        check_remove(m.to, history[tesuu]);
    
    }else{  //drawの場合
        doDraw(history[tesuu]);
    }

    //TODO: 山片づけのHistory記録
    getHash(history[tesuu].md5sum);
    tesuu++;

    //print();
}

/****************************************************************************/
void Board::doDraw(History &h)
{
    //printf("### doDraw ###\n");
    assert(stock_remain>=1);
    
    
    for( int x=0; x<WIDTH; x++){
        //既存の枚数チェック、その上に配る
        int y;
        for( y=0; ; y++ ){
            if( tableau[x][y].n == 0 ){
                assert(y>=1);
                tableau[x][y] = stock[x][stock_remain-1];
                break;
            }
        }
    }
    stock_remain--;
    //TODO: 山片づけチェック
}
/****************************************************************************/
void Board::check_remove(int x, History &h)
{
    Katamari k(tableau[x]);
    if( k.top==1 && k.bottom==13 ){
        //printf("Removing x=%d\n", x);
        for( int y=k.position; y<k.position+13; y++){
            tableau[x][y].n=0;
        }
        h.remove_x[h.remove_num] = x;
        h.remove_suit[h.remove_num] = k.s;
        h.removepile_fliped[h.remove_num] = false;
        
        //その下のカードを開く
        if( k.position>=1 ){
            if( tableau[x][k.position-1].invisible ){
                if( tableau[x][k.position-1].n == card_unknown ){
                    inquire(x, k.position-1);
                }
                tableau[x][k.position-1].invisible = false;
                h.removepile_fliped[h.remove_num] = true;
            }
        }
        h.remove_num++;
        kataduke++;
        //print();
    }
}
/****************************************************************************/
void Board::undo()
{
    //printf("### undo ###\n");
    
    assert(tesuu>=1);
    History &h = history[tesuu-1];

    for( int i=0; i<h.remove_num; i++ ){
        //print();
        //printf("undo 片付け\n");
        int x=h.remove_x[i];
        int y;
        for( y=0; tableau[x][y].n!=card_empty; y++ ){} //高さ

        if( h.removepile_fliped[i] ){
            assert(y>=1);
            tableau[x][y-1].invisible = true;
        }
            
        for( int n=13; n>=1; n--, y++){
            tableau[x][y].n    = n;
            tableau[x][y].suit = h.remove_suit[i];
            tableau[x][y].invisible = false;
        }
        kataduke--;
        //print();
    }

    if( h.m.isDraw() ){
        //printf("undoing draw.\n");
        //print();
        undo_draw();
        //print();
        //printf("undoing draw. done.\n");
    }else{

        //移動先(undoの移動元)の高さチェック
        int top_y = -1;
        for( int y=0; ; y++){ //topのyを算出
            if( tableau[h.m.to][y].n == card_empty ){
                top_y = y-1;
                break;
            }
        }
        assert(tableau[h.m.to][top_y].n == h.m.k.top);

        if( h.frompile_fliped ){
            //移動元のtopを裏にする。
            assert(h.m.k.position>=1);
            tableau[h.m.from][h.m.k.position-1].invisible = true;
        }

        //塊の逆移動
        for( int i=h.m.k.bottom-h.m.k.top; i>=0 ;i--){
            assert(top_y>=0);
            tableau[h.m.from][h.m.k.position+i] = tableau[h.m.to][top_y];
            tableau[h.m.to][top_y].n = card_empty;
            top_y--;
        }
    }
    tesuu --;
}
/****************************************************************************/
void Board::undo_draw()
{
    for( int x=0; x<WIDTH; x++){
        //高さチェック
        int top_y = 0;
        for( int y=0; ; y++){
            if( tableau[x][y].n==card_empty ){
                top_y = y-1;
                break;
            }
        }
        assert(top_y>=1);
        assert(tableau[x][top_y].n==stock[x][stock_remain].n);
        tableau[x][top_y].n = card_empty;
    }
    stock_remain++;
}
/****************************************************************************/
void solve(Board &board)
{
    static long count =0;
    count++;
    if( count%65536==0){
        printf("now examing pattern No.%ld...", count);
        board.print();
    }
    
    if( board.isComplete() ){
        board.print();
        printf("Conguraturation!! examined boards=%ld\n", count);
        exit(0);
    }
    
    Candidate  candidate[100];
    int   num;
    board.search_candidate(candidate, &num);

    for( int i=0; i<num; i++ ){
        board.doMove(candidate[i].m);
        solve(board);
        //printf("###undoing. tesuu=%d, %d/%d\n", board.tesuu, i+1,num);
        board.undo();
    }
}
/****************************************************************************/
#ifndef TEST
int main(int argc, const char* argv[])
{
    Board board;
    
    board.init(argc-1, argv+1);
    board.print();
    solve(board);
    printf("Sorry, No answer.\n");
    return 0;
}
#else
/****************************************************************************/
//テスト関数
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
/****************************************************************************/
class FunctionTest : public CPPUNIT_NS::TestFixture{
    //テストクラス
    CPPUNIT_TEST_SUITE(FunctionTest);//登録のスタート
    CPPUNIT_TEST(test_test);
    CPPUNIT_TEST(test_search_candidate);
    CPPUNIT_TEST_SUITE_END();//登録の終了

protected:
    void test_test();
    void test_search_candidate();

public:
    void setUp();
    void testDown();
};
/****************************************************************************/
CPPUNIT_TEST_SUITE_REGISTRATION(FunctionTest);

//テスト起動時に実行
void FunctionTest::setUp(){
}

//テスト終了時に実行
void FunctionTest::testDown(){
}
/****************************************************************************/
void FunctionTest::test_test()
{
    Board board;

    CPPUNIT_ASSERT_EQUAL(true, board.isComplete());
    CPPUNIT_ASSERT_EQUAL(true, board.existEmpty());
    board.tableau[0][0] = Card();
    CPPUNIT_ASSERT_EQUAL(false, board.isComplete());
    board.print();
    
    const char* argv[12] = {"xs3 s2 s1"," s1"," s2"," s3"," s4"," s5"," s6"," s7"," s8"," s9","-","s1s2s3s4s5s6s7s8s9s0"};
    board.init(12,argv);
    CPPUNIT_ASSERT_EQUAL(3,board.tableau[0][0].n);
    CPPUNIT_ASSERT_EQUAL(1,board.tableau[0][2].n);
    board.print();
    
    Katamari k(board.tableau[0]);
    CPPUNIT_ASSERT_EQUAL(spade, k.s);
    CPPUNIT_ASSERT_EQUAL(1, k.top);
    CPPUNIT_ASSERT_EQUAL(2, k.bottom);
    CPPUNIT_ASSERT_EQUAL(1, k.position);
    
    Candidate  candidate[100];
    int   num;
    board.search_candidate(candidate, &num);
    CPPUNIT_ASSERT_EQUAL(10,num);

    board.doMove(candidate[0].m);
    board.print();
    board.undo();
    board.print();
}
/****************************************************************************/
void FunctionTest::test_search_candidate()
{
    Board board;
    
}
/****************************************************************************/
int main(int argc, char* argv[])
{
    CPPUNIT_NS::TestResult controller;

    //結果収集
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    //途中結果の収集
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    //テストを走らせる。テストを入れて走る
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run(controller);

    //結果を標準出力にする。
    CPPUNIT_NS::CompilerOutputter outputter(&result,CPPUNIT_NS::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}

#endif
