/**
 *  main.c
 *  lame-obj-c
 *
 *  Created by Daniel Drzimotta on 2012-09-05.
 *  Copyright (c) 2012 Daniel Drzimotta. All rights reserved.
 */

#include <stdio.h>

#include "lame-obj-c.h"
#include <string.h>

void AutoReleaseTest0();
void AutoReleaseTest1();
void AutoReleaseTest2();
void AutoReleaseTest3();
void AutoReleaseTest4();

void StringAndConsTest0();

int main(int argc, const char * argv[])
{
    printf("Starting App\n");
    SetupObjectSystem();
    
    AutoReleasePoolCreate();
        
    AutoReleaseTest0();
    AutoReleaseTest1();
    AutoReleaseTest2();
    AutoReleaseTest3();
    AutoReleaseTest4();

    StringAndConsTest0();
    
    AutoReleasePoolDrain();
    
    printf("Number of cons cells made: %u\n", numberOfConsCreated());
    printf("Number of leaked cons cells: %u\n", numberOfLeakedCons());
    
    printf("Ending App\n");
    return 0;
}

void AutoReleaseTest0() {
    printf("Starting AutoRelease Test 0\n");
    AutoReleasePoolCreate();
    ConsRef cons0 = ConsCreate(nil, nil);
    AutoRelease(cons0);
    ConsRef cons1 = ConsCreate(nil, nil);
    AutoRelease(cons1);
    ConsRef cons2 = ConsCreate(nil, nil);
    AutoRelease(cons2);
    
    AutoReleasePoolDrain();
    printf("Ending AutoRelease Test 0\n");
}


void AutoReleaseTest1() {
    printf("Starting AutoRelease Test 1\n");
    AutoReleasePoolCreate();
    
    ConsRef cons = ConsCreate(nil, nil);
    AutoRelease(cons);
    Retain(cons);
    AutoRelease(cons);
    Retain(cons);
    AutoRelease(cons);
    
    
    AutoReleasePoolCreate();
    ConsRef cons2 = ConsCreate(nil, nil);
    AutoRelease(cons2);
    Retain(cons2);
    AutoRelease(cons2);
    Retain(cons2);
    AutoRelease(cons2);
    AutoReleasePoolDrain();
    
    ConsRef test = ConsCreate(nil, nil);
    AutoRelease(test);
    
    AutoReleasePoolDrain();
    printf("Ending AutoRelease Test 1\n");
}

void AutoReleaseTest2() {
    printf("Starting AutoRelease Test 2\n");
    AutoReleasePoolCreate();
    ConsRef cons = ConsCreate(AutoRelease(ConsCreate(nil, nil)),
                              AutoRelease(ConsCreate(nil, nil)));
    AutoRelease(cons);
    Retain(cons);
    AutoRelease(cons);
    Retain(cons);
    AutoRelease(cons);
    
    
    AutoReleasePoolCreate();
    ConsRef cons2 = ConsCreate(AutoRelease(ConsCreate(nil, nil)),
                               nil);
    AutoRelease(cons2);
    Retain(cons2);
    AutoRelease(cons2);
    Retain(cons2);
    AutoRelease(cons2);
    AutoReleasePoolDrain();
    
    ConsRef test = ConsCreate(nil, nil);
    AutoRelease(test);
    
    AutoReleasePoolDrain();
    printf("Ending AutoRelease Test 2\n");
}

void AutoReleaseTest3() {
    printf("Starting AutoRelease Test 3\n");
    
    /* Lets thrash our autorelease pool here... */
    AutoReleasePoolCreate();
    
    int numberOfOuterCons = 99;
    int numberOfInnerCons = 99;
    int i = 0;
    int j = 0;

    for (i = 0; i < numberOfOuterCons; i++) {
        ConsRef newCons = ConsCreate(nil, nil);
        AutoRelease(newCons);
        
        for (j = 0; j < numberOfInnerCons; j++) {
            AutoReleasePoolCreate();
            ConsRef newNewCons = ConsCreate(nil, nil);
            AutoRelease(newNewCons);
            newNewCons = ConsPush(newNewCons, AutoRelease(ConsCreate(AutoRelease(ConsCreate(AutoRelease(ConsCreate(nil, nil)),
                                                                                            AutoRelease(ConsCreate(nil, nil)))),
                                                                     AutoRelease(ConsCreate(AutoRelease(ConsCreate(nil, nil)),
                                                                                            AutoRelease(ConsCreate(nil, nil)))))));
            AutoReleasePoolDrain();
        }
    }
    
    AutoReleasePoolDrain();
    printf("Ending AutoRelease Test 3\n");
}

void AutoReleaseTest4() {
    ConsRef testCons = ConsCreate(AutoRelease(ConsCreate(nil, nil)),
                                  AutoRelease(ConsCreate(nil, nil)));
    AutoRelease(testCons);
}

void StringAndConsTest0() {
    printf("Starting String Test 0\n");
    
    CharRef aChar = CharCreate('a');
    StringPrint(aChar, "aChar (a): %s\n");
    Release(aChar);
    
    StringRef aString = StringCreate("hello");
    StringPrint(aString, "aString (hello): %s\n");
    
    BOOL isEqual = StringEqual(aString, AutoRelease(StringCreate("hello")));
    printf("Equal: (YES): %s\n", isEqual ? "YES" : "NO");
    
    BOOL isNotEqual = StringEqual(aString, AutoRelease(StringCreate("noHello")));
    printf("Equal: (NO): %s\n", isNotEqual ? "YES" : "NO");
    Release(aString);
    
    ConsRef aCons = ConsCreate(nil, nil);
    ConsRef bCons = ConsCreate(AutoRelease(StringCreate("abcdr")), nil);
    ConsRef cCons = ConsCreate(nil, AutoRelease(StringCreate("wakka")));
    ConsRef dCons = ConsCreate(AutoRelease(StringCreate("abc")), AutoRelease(StringCreate("def")));
    ConsRef eCons = ConsCreate(AutoRelease(StringCreate("a")),
                               AutoRelease(ConsCreate(AutoRelease(StringCreate("b")),
                                                      AutoRelease(ConsCreate(AutoRelease(StringCreate("c")),
                                                                             nil)))));
    
    StringPrint(aCons, "aCons '()': %s\n");
    StringPrint(bCons, "bCons '(\"abcdr\")': %s\n");
    StringPrint(cCons, "cCons '(NIL . \"wakka\")': %s\n");
    StringPrint(dCons, "dCons '(\"abc\" . \"def\")': %s\n");
    StringPrint(eCons, "eCons '(\"a\" \"b\" \"c\")': %s\n");
    Release(aCons);
    Release(bCons);
    Release(cCons);
    Release(dCons);
    Release(eCons);
    
    
    ConsRef fCons = ConsCreate(AutoRelease(CharCreate('a')),
                               AutoRelease(ConsCreate(AutoRelease(ConsCreate(AutoRelease(CharCreate('q')),
                                                                             AutoRelease(ConsCreate(AutoRelease(CharCreate('w')),
                                                                                                    AutoRelease(ConsCreate(AutoRelease(CharCreate('z')),
                                                                                                                           nil)))))),
                                                      AutoRelease(ConsCreate(AutoRelease(ConsCreate(nil,
                                                                                                    AutoRelease(CharCreate('c')))),
                                                                             AutoRelease(ConsCreate(AutoRelease(CharCreate('d')),
                                                                                                    AutoRelease(StringCreate("Boop.")))))))));
    
    StringPrint(fCons, "fCons '(a (q w z) (NIL . c) d . \"Boop.\")': %s\n");
    Release(fCons);
    
    
    
    printf("Ending String Test 0\n");
}
