
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
class Card{
public:
    
    Suit  suit;
    int   n;
    bool  invisible;
    
    Card(): suit(spade),n(1){}
    Card(Suit s, int n_): suit(s), n(n_), invisible(true){}

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
        if( card_array[y].suit != s ){
            break;
        }
        if( card_array[y].n != card_array[y+1].n+1 ){
            break;
        }
    }
    position = y+1;
    bottom   = card_array[position].n;
}
/****************************************************************************/
class Board{
public:
    enum{
        WIDTH = 10, STOCK_MAX=5,
    };

    Card  tableau[WIDTH][104];
    int   tesuu;
    Card  stock[WIDTH][5];

    Board(){init();}
    void init();
    void init(int argc, const char* argv[]);  //mainに渡されるものからargv[0]を除いて渡すこと
    void print()const;

    bool isComplete()const{for(int x=0;x<WIDTH;x++)if(tableau[x][0].n!=0)return false; return true;}
    bool existEmpty()const{for(int x=0;x<WIDTH;x++)if(tableau[x][0].n==0)return true; return false;}
    bool canMove(Move m)const;
    
    void search_candidate(Move *candidate, int *num)const;
    
    void doMove(const Move &m);
    void check_remove();
    void check_remove(int x);
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
}
/****************************************************************************/
void Board::init(int argc, const char* argv[])
{
    assert( argc>=11 );
    init();
    
    //場の設定
    for( int x=0; x<WIDTH; x++){
        size_t len = strlen(argv[x])/2;
        for( size_t y=0; y<len; y++ ){
            tableau[x][y].n    = c2i(argv[x][y*2+1]);
            tableau[x][y].suit = c2s(argv[x][y*2]);
        }
    }

    //ストックの設定
    assert(strcmp(argv[WIDTH],"-")==0);
    for( int i=WIDTH+1; i<argc; i++){
        assert(strlen(argv[i])==WIDTH*2);
        for( int x=0; x<WIDTH; x++){
            stock[x][i].n    = c2i(argv[i][x*2+1]);
            stock[x][i].suit = c2s(argv[i][x*2]);
        }
    }
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
                printf(" =%c%c= ",suit_char[tableau[x][y].suit],
                                 " 1234567890JQK*"[tableau[x][y].n]);
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
    printf("tesuu=%d\n", tesuu);
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
    
    for( int from=0; from<WIDTH; from++){
        for( int to=0; to<WIDTH; to++){
            if( from==to ){continue;}
            if( k[from].isEmpty() || k[to].isEmpty() ){continue;}
            if( k[from].bottom +1 == k[to].top ){
                candidate[*num].from = from;
                candidate[*num].to   = to;
                candidate[*num].k    = k[from];
                //candidate[*num].num  = 0;  //all katamari move
                (*num)++;
            }
        }
    }
    printf("candidate num=%d\n", *num);
}
/****************************************************************************/
void Board::doMove(const Move &m)
{
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
    tesuu++;
    
    //除去チェック
    check_remove(m.to);
}

/****************************************************************************/
void Board::check_remove(int x)
{
    Katamari k(tableau[x]);
    if( k.top==1 && k.bottom==13 ){
        printf("Removing x=%d\n", x);
        for( int y=k.position; y<k.position+13; y++){
            tableau[x][y].n=0;
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
    
    const char* argv[12] = {"s3s2s1","s1","s2","s3","s4","s5","s6","s7","s8","s9","-","s1s2s3s4s5s6s7s8s9s0"};
    board.init(12,argv);
    CPPUNIT_ASSERT_EQUAL(3,board.tableau[0][0].n);
    CPPUNIT_ASSERT_EQUAL(1,board.tableau[0][2].n);
    board.print();
    
    Katamari k(board.tableau[0]);
    CPPUNIT_ASSERT_EQUAL(spade, k.s);
    CPPUNIT_ASSERT_EQUAL(1, k.top);
    CPPUNIT_ASSERT_EQUAL(3, k.bottom);
    CPPUNIT_ASSERT_EQUAL(0, k.position);
    
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
