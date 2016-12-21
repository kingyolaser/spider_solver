
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

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
    
    Card(): suit(spade),n(1),invisible(false){}
    Card(Suit s, int n_): suit(s), n(n_), invisible(false){}

};

/****************************************************************************/
class Katamari{
public:
    Suit   s;
    int    position;  //bottomの配列index
    int    top;
    int    bottom;
    
    Katamari():s(spade),position(0),top(0),bottom(0){}
    Katamari(const Card* card_array):s(spade),position(0),top(0),bottom(0){init(card_array);}
    void init(const Card* card_array);
    
    bool isEmpty()const{return top==0;}
};
/****************************************************************************/
class Move{
public:
    int from;  //-1 means draw
    int to;
    //int num;   //塊を分割して、emptyへ移動させるときの枚数。toがemptyでなければ0を入れる。
    Katamari k;
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
#define MAX_REMOVE 7  //１手での最大片付山数。draw時に配ったカードで同時に片づけられたケース。
class History{
public:
    Move m;
    bool frompile_fliped;
    int  remove_num;
    int  remove_x[MAX_REMOVE];
    Suit remove_suit[MAX_REMOVE];
    bool removepile_fliped[MAX_REMOVE]; //除去後にその下をめくったかどうか
    
    void init(){ frompile_fliped=false; remove_num=0; }
};
/****************************************************************************/
class Board{
public:
    enum{
        WIDTH = 10, STOCK_MAX=5, HISTORY_MAX=300,
    };

    Card  tableau[WIDTH][104];
    int   tesuu;
    Card  stock[WIDTH][5];
    int   stock_remain;
    History  history[HISTORY_MAX];

    Board(){init();}
    void init();
    void init(int argc, const char* argv[]);  //mainに渡されるものからargv[0]を除いて渡すこと
    void print()const;

    bool isComplete()const{for(int x=0;x<WIDTH;x++)if(tableau[x][0].n!=0)return false; return true;}
    bool existEmpty()const{return getEmpty()!=-1;}
    int  getEmpty()const{for(int x=0;x<WIDTH;x++)if(tableau[x][0].n==0)return x; return -1;}
    bool canMove(Move m)const;
    
    void search_candidate(Move *candidate, int *num)const;
    
    void inquire(int x, int y);
    void doMove(const Move &m);
    void doDraw(History &h);
    void check_remove(int x, History &h);
    void undo();
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
            stock[x][i-WIDTH-1].n    = c2i(argv[i][x*2+1]);
            stock[x][i-WIDTH-1].suit = c2s(argv[i][x*2]);
        }
    }
    stock_remain = argc-WIDTH-1;
}
/****************************************************************************/
void Board::print() const
{
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
                }
                exist = true;
            }else{
                printf("  %c%c  ",suit_char[tableau[x][y].suit],
                                 " 1234567890JQK*"[tableau[x][y].n]);
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
        }
    }
    
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
void Board::search_candidate(Move *candidate, int *num)const
{
    Katamari k[WIDTH];
    
    *num = 0;
    for( int from=0; from<WIDTH; from++){
        k[from].init(tableau[from]);
        //printf("%d-%d\n", k[from].top, k[from].bottom);
    }
    
    //塊全体移動のチェック。suit違っても候補。
    for( int from=0; from<WIDTH; from++){
        for( int to=0; to<WIDTH; to++){
            if( from==to ){continue;}
            if( k[from].isEmpty() || k[to].isEmpty() ){continue;}
            if( k[from].bottom +1 == k[to].top ){
                candidate[*num].from = from;
                candidate[*num].to   = to;
                candidate[*num].k    = k[from];
                (*num)++;
            }
        }
    }
    
    //kingからの山に積み重ねられるかチェック
    //塊全体でなくてよい。同一suit必須
    for( int to=0; to<WIDTH; to++){
        if( k[to].bottom==13 ){
            for( int from=0; from<WIDTH; from++){
                if( from==to ){ continue; }
                if( k[from].bottom ==13 ){continue;}
                if( k[from].top<k[to].top && k[to].top <= k[from].bottom ){
                    //↑この条件で、塊全体移動は除外されている。
                    Katamari partial_k = k[from];
                    partial_k.bottom   = k[to].top-1;
                    partial_k.position = k[from].position + (k[from].bottom - k[to].top+1);
                    
                    candidate[*num].from = from;
                    candidate[*num].to   = to;
                    candidate[*num].k    = partial_k;
                    (*num)++;
                    printf("from=%d, to=%d, %d-%d\n", from, to, partial_k.top, partial_k.bottom);
                    //exit(0);
                }
            }
        }
    }
    
    //empty spaceがある場合、塊を下す手を列挙
    //
    int x;
    if( (x=getEmpty())!=-1 ){
        for( int from=0; from<WIDTH; from++ ){
            if( k[from].position != 0 ){ //塊は下す価値がある
                candidate[*num].from = from;
                candidate[*num].to   = x;
                candidate[*num].k    = k[from];
                (*num)++;
            }
        }
    }
    
    if( stock_remain>=1 && !existEmpty() ){
        candidate[*num].from = -1;
        (*num)++;
    }

    printf("candidate num=%d\n", *num);
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
        printf("Moving: %d->%d\n", m.from, m.to);
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
                    inquire(m.from, m.k.position-1);
                }
                tableau[m.from][m.k.position-1].invisible = false;
            }
        }
    
        //除去チェック
        check_remove(m.to, history[tesuu]);
    
    }else{  //drawの場合
        doDraw(history[tesuu]);
    }

    //TODO: 山片づけのHistory記録
    tesuu++;

    print();
}

/****************************************************************************/
void Board::doDraw(History &h)
{
    printf("### doDraw ###\n");
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
        printf("Removing x=%d\n", x);
        for( int y=k.position; y<k.position+13; y++){
            tableau[x][y].n=0;
        }
        
        //その下のカードを開く
        if( k.position>=1 ){
            if( tableau[x][k.position-1].invisible ){
                if( tableau[x][k.position-1].n == card_unknown ){
                    inquire(x, k.position-1);
                }
                tableau[x][k.position-1].invisible = false;
                h.frompile_fliped = true;
            }
        }
    }
}
/****************************************************************************/
void Board::undo()
{
    //TODO
    printf("undo not supported yet\n");
    exit(1);
}
/****************************************************************************/
void solve(Board &board)
{
    board.print();
    
    if( board.isComplete() ){
        printf("Conguraturation!!\n");
        exit(0);
    }
    
    Move  candidate[100];
    int   num;
    board.search_candidate(candidate, &num);

    for( int i=0; i<num; i++ ){
        board.doMove(candidate[i]);
        solve(board);
        board.undo();
    }
}
/****************************************************************************/
#ifndef TEST
int main(int argc, const char* argv[])
{
    Board board;
    
    board.init(argc-1, argv+1);
    solve(board);
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
    
    Move  candidate[100];
    int   num;
    board.search_candidate(candidate, &num);
    CPPUNIT_ASSERT_EQUAL(9,num);
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
