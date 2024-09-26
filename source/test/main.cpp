//#include "Inter/Interpolator.h"
#include "test/InterCXX.hpp"
#include "locale"
#include <string>
#include <iostream>
extern "C"{
#include "Inter/common.h"
}
void test_set_get()
{
    const sreal_t dv[3]		= { 1 ,    2   ,  3  };
	const sreal_t al[2]		= { -3  ,   4  };
	const sreal_t M[2]		= { -1   ,  1  };
	const sreal_t bet[3]	= { -0.88  ,   0   ,  0.88  };
    const sreal_t dt[36]    = {1   ,    2   ,    3    ,   
4   ,    5  ,     6    ,   
7     ,  8      , 9      , 
10    ,   11     ,  12      , 
13    ,   14     ,  15       ,
16    ,   17     ,  18       ,
19    ,   20     ,  21       ,
22   ,    23     ,  24       ,
25    ,   26     ,  27       ,
28    ,   29     ,  30       ,
31    ,   32     ,  33       ,
34    ,   35     ,  36 };
    science::Interpolator icz;
    icz.make_data(4);
    icz.set_argument(0, dv,  3);
    icz.set_argument(1, al,  2);
    icz.set_argument(2, M,   2);
    icz.set_argument(3, bet, 3);
    icz.set_values(dt, 36);
    
    displayDisriptionOfTable(); 
    displayScales();
    displayDisriptionOfData();
    displayData();

    //icx->dump(L"D:\\Blocknot\\newTest\\OUT", BINARY);
    std::cout << "\n";
    for(sreal_t al = 1; al <= 3; al+= 1){
       for(sreal_t a2 = 3; a2 <= 4; a2 += 1)
           for(sreal_t a3 = -1; a3 <= 1; a3 += 1)
               for(sreal_t de = -0.88; de <= 0.88; de += 0.88 ){
                   std::cout <<"al: "<< al <<" | a2: " << a2 << " | a3: "<< a3 <<" | de: " << de 
                   << " | " <<icz(al, a2, a3, de) << "\n";
               }
       std::cout << "\n";
    }
    icz.dump(L"D:\\Blocknot\\newTest\\OUT_TEXT", TEXT);
    icz.dump(L"D:\\Blocknot\\newTest\\OUT_BINAR", BINARY);
}
int main()
{
    test_set_get();
    //return -9;
    //setlocale(LC_ALL, "en_US.utf8");
    const wchar_t * p = L"D:\\Projects\\MM\\MM65\\data\\vehicle\\T10\\cy.dat";//L"D:\\Blocknot\\newTest\\formated.mz.m.bet.al.dh.dat";//L"D:\\Blocknot\\res\\re\\cx.de.dat";//L"D:\\Blocknot\\АБВ.txt";
    const wchar_t * pb = L"D:\\Blocknot\\newTest\\OUT_BINAR.lid"; 
    const wchar_t * pt = L"D:\\Blocknot\\for.dat";
    const wchar_t * pt2 = L"D:\\Projects\\MM\\MM65\\data\\vehicle\\T10\\al_max.dat";
    science::Interpolator icx(pb, BINARY);
    
    displayDisriptionOfTable(); 
    displayScales();
    displayDisriptionOfData();
    displayData();

    //icx->dump(L"D:\\Blocknot\\newTest\\OUT", BINARY);
    std::cout << "\n";
    for(sreal_t al = 1; al <= 3; al+= 1){
       for(sreal_t a2 = 3; a2 <= 4; a2 += 1)
           for(sreal_t a3 = -1; a3 <= 1; a3 += 1)
               for(sreal_t de = -0.88; de <= 0.88; de += 0.88 ){
                   std::cout <<"al: "<< al <<" | a2: " << a2 << " | a3: "<< a3 <<" | de: " << de 
                   << " | " <<icx(al, a2, a3, de) << "\n";
               }
       std::cout << "\n";
    }
    // for(sreal_t al = -6; al <= 10; al+= 2){
    //     for(sreal_t de = -10; de <= 10; de+= 5)
    //         for(sreal_t M = 0.4; M <= 0.9; M+= 0.1)
    //             for(sreal_t bet = -6; bet <= 6; bet += 6)
    //                 std::cout << " | " <<icx(al,de,M,bet);
    //     std::cout << "\n";}
    icx.dump(L"D:\\Blocknot\\newTest\\OUT_TEXT", TEXT);
    icx.dump(L"D:\\Blocknot\\newTest\\OUT_BINAR", BINARY);
    int t = 0;
}