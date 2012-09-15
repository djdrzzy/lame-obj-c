/**
 *  lame-obj-c.c
 *  lame-obj-c
 *
 *  Created by Daniel Drzimotta on 2012-09-05.
 *  Copyright (c) 2012 Daniel Drzimotta. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lame-obj-c.h"

#pragma mark Base Object System

static ConsRef _autoReleasePools = nil;
static ssize_t *registeredTypes = NULL;

static const ObjectTypeIdentifier = 0;
static const ConsTypeIdentifier = 1;
static const AutoReleasePoolTypeIdentifier = 2;
static const CharTypeIdentifier = 3;
static const StringTypeIdentifier = 4;

void _AutoReleasePoolRegister(Object obj);
ConsRef _ConsPop(ConsRef cons, Object *obj, BOOL shouldAutoRelease);
ConsRef _ConsPush(ConsRef cons, Object obj, BOOL shouldAutoRelease);
ObjectType _Kind(Object obj);

typedef struct ObjectState {
    ObjectType kind;
    RefCount refCount;
    DeallocFunc deallocFunc;
    DescriptionFunc descriptionFunc;
} ObjectState;

typedef struct ConsRefState {
    ObjectState common;
    Object car;
    Object cdr;
} ConsRefState;

typedef struct AutoReleasePoolRefState {
    ObjectState common;
    ConsRef *objectsToRelease;
} AutoReleasePoolRefState;

typedef struct CharRefState {
    ObjectState common;
    char character;
} CharRefState;

typedef struct StringRefState {
    ObjectState common;
    ConsRef consRep;
} StringRefState;

#pragma mark Utility

void RegisterObjectType(ObjectType type, ssize_t objectSize) {
    
    ssize_t existsAlready = registeredTypes[type];
    if (existsAlready) {
        printf("You tried to register a type (%i) of object that already exists.\n", type);
        abort();
    }
    
    registeredTypes[type] = objectSize;
}

BOOL RegisteredObjectTypeExists(ObjectType type) {
    return registeredTypes[type] ? YES : NO;
}

ssize_t RegisteredObjectSize(ObjectType type) {
    if (!RegisteredObjectTypeExists(type)) {
        printf("That type of object (%i) has not been registered.\n", type);
    }
    
    return registeredTypes[type];
}

void SetupObjectSystem() {
    registeredTypes = calloc(1, kTypesOfObjectsAllowed);
    RegisterObjectType(ObjectTypeIdentifier, sizeof(ObjectState));
    RegisterObjectType(ConsTypeIdentifier, sizeof(ConsRefState));
    RegisterObjectType(AutoReleasePoolTypeIdentifier, sizeof(AutoReleasePoolRefState));
    RegisterObjectType(CharTypeIdentifier, sizeof(CharRefState));
    RegisterObjectType(StringTypeIdentifier, sizeof(StringRefState));
}

Object _ObjectInitialize(ObjectType type, DeallocFunc deallocFunc, DescriptionFunc descriptionFunc) {
    
    Object obj = calloc(1, RegisteredObjectSize(type));
    ObjectState *common = (ObjectState*)obj;
    common->kind = type;
    common->deallocFunc = deallocFunc;
    common->descriptionFunc = descriptionFunc;
    Retain(obj);
    if (!obj) {
        printf("ERROR Creating obj.\n");
        abort();
    }
    return obj;
}

ObjectType _Kind(Object obj) {
    if (obj) {
        ObjectState *common = (ObjectState*)obj;
        return common->kind;
    } else {
        return -1;
    }
    
}

void _abortIfMismatch(Object obj, ObjectType type) {
    if (_Kind(obj) != type) {
        abort();
    }
}

#pragma mark AutoReleasePool

void _AutoReleasePoolDealloc(Object obj) {
    AutoReleasePoolRefState *refState = (AutoReleasePoolRefState*)obj;
    Release(refState->objectsToRelease);
    refState->objectsToRelease = nil;
}

StringRef _AutoReleasePoolDescription(Object obj) {
    char *description = calloc(1, 100);
    sprintf(description, "Autorelease pool: %p", obj);
    StringRef stringToReturn = StringCreate(description);
    free(description);
    return AutoRelease(stringToReturn);
}



void _AutoReleasePoolRegister(Object obj) {
    AutoReleasePoolRefState *currentPool = ConsCar(_autoReleasePools);
    if (!currentPool) {
        printf("AutoReleasing with no pool in place. Leaking memory.\n");
        return;
    }
    ConsRef oldObjectsToRelease = currentPool->objectsToRelease;
    currentPool->objectsToRelease = _ConsPush(currentPool->objectsToRelease, obj, NO);
    Release(oldObjectsToRelease);
    Release(obj);
}

void AutoReleasePoolCreate() {
    
    AutoReleasePoolRefState *newPool = _ObjectInitialize(AutoReleasePoolTypeIdentifier,
                                                         &_AutoReleasePoolDealloc,
                                                         &_AutoReleasePoolDescription);
    
    
    ConsRef oldAutoReleasePool = _autoReleasePools;
    
    _autoReleasePools = _ConsPush(_autoReleasePools, newPool, NO);
    
    Release(oldAutoReleasePool);
    
    Release(newPool);
}

void AutoReleasePoolDrain() {
    Object obj = nil;
    AutoReleasePoolRef oldStack = _autoReleasePools;
    _autoReleasePools = _ConsPop(_autoReleasePools, &obj, NO);
    
    Release(oldStack);
}

#pragma mark Memory

void Retain(Object obj) {
    if (obj) {
        ObjectState *common = (ObjectState *)obj;
        common->refCount += 1;
    }
}

RefCount RetainCount(Object obj) {
    if (obj) {
        ObjectState *common = (ObjectState *)obj;
        return common->refCount;
    }
    return 0;
}

void Release(Object obj) {
    if (obj) {
        ObjectState *common = (ObjectState *)obj;
        common->refCount -= 1;
        if (common->refCount == 0) {
            common->deallocFunc(obj);
            free(obj);
        }
    }
}

Object AutoRelease(Object obj) {
    if (obj) {
        _AutoReleasePoolRegister(obj);
    }
    
    return obj;
}

StringRef Description(Object obj) {
    char *notFound = "Description method not found.";
    StringRef notFoundRef = AutoRelease(StringCreate(notFound));
    if (obj) {
        ObjectState *common = (ObjectState *)obj;
        return common->descriptionFunc ? common->descriptionFunc(obj) : notFoundRef;
    } else {
        return nil;
    }
}


#pragma mark Cons
static unsigned int _consMade = 0;
static unsigned int _consDealloced = 0;

unsigned int numberOfConsCreated() {
    return _consMade;
}
unsigned int numberOfLeakedCons() {
    return _consMade - _consDealloced;
}

void _ConsDealloc(Object obj) {
    _consDealloced++;
    
    ConsRefState *cons = (ConsRefState *)obj;
    ConsSetCar(cons, nil);
    ConsSetCdr(cons, nil);
}

StringRef _ConsDescriptionWithOpeningPar(Object obj, BOOL openingPar) {
    ConsRef *cons = obj;
    
    if (_Kind(obj) != ConsTypeIdentifier) {
            return AutoRelease(Description(cons));
    }
    
    
    if (!ConsCar(cons) && !ConsCdr(cons)) {
        return AutoRelease(StringCreate("()"));
    }
    
    Object carVal = ConsCar(cons) ?: AutoRelease(StringCreate("NIL"));
    Object cdrVal = ConsCdr(cons);
    
    StringRef openingParString = openingPar ? StringCreate("(%s") : StringCreate(" %s");
    
    if (ConsCdr(cons)) {
        StringConcatenate(openingParString, AutoRelease(StringCreate(" ")));
    }
    
    StringRef carDescription = Description(carVal);
    
    if (ConsCar(cons) && _Kind(carVal) == StringTypeIdentifier) {
        carDescription = StringSPrint(carDescription, "\"%s\"");
    }
    
    char *fetchedCString =   StringCString(carDescription);
    char *fetchedParString = StringCString(Description(openingParString));
    char *aCar = calloc(1, (fetchedCString ? strlen(fetchedCString) : 0) + (fetchedParString ? strlen(fetchedParString) : 0));
    sprintf(aCar, fetchedParString, fetchedCString);
    free(fetchedCString);
    free(fetchedParString);
    
    char *aCdr = NULL;
    
    if (cdrVal) {
        
        if (_Kind(cdrVal) == ConsTypeIdentifier) {
            char *fetchedCString = StringCString(_ConsDescriptionWithOpeningPar(cdrVal, NO));
            aCdr = fetchedCString;
        } else {
            
            StringRef cdrDescription = _ConsDescriptionWithOpeningPar(cdrVal, NO);
            
            if (_Kind(cdrVal) == StringTypeIdentifier) {
                cdrDescription = StringSPrint(cdrDescription, "\"%s\"");
            }
            
            StringRef catted = StringConcatenate(AutoRelease(StringCreate(" . ")), cdrDescription);
            catted = StringConcatenate(catted, AutoRelease(StringCreate(")")));
            aCdr = StringCString(catted);
        }
        
        
    } else {
        aCdr = calloc(1, strlen(")"));
        sprintf(aCdr, "%s", ")");
    }
    
    char *returnString = calloc(1, 1 + strlen(aCar) + strlen(aCdr));
    sprintf(returnString, "%s%s", aCar, aCdr);
    
    Release(openingParString);
    free(aCar);
    free(aCdr);
    
    StringRef rVal = AutoRelease(StringCreate(returnString));
    
    
    free(returnString);
    
    return rVal;
}

StringRef _ConsDescription(Object obj) {
    return _ConsDescriptionWithOpeningPar(obj, YES);
}

ConsRef ConsCreate(Object car, Object cdr) {
    _consMade++;
    
    ConsRef newCons = _ObjectInitialize(ConsTypeIdentifier, &_ConsDealloc, &_ConsDescription);
    ConsSetCar(newCons, car);
    ConsSetCdr(newCons, cdr);
    return newCons;
}

Object ConsCar(ConsRef cons) {
    _abortIfMismatch(cons, ConsTypeIdentifier);
    
    return cons ? ((ConsRefState*)cons)->car : nil;
}

void ConsSetCar(ConsRef cons, Object obj) {
    Object prvObj = ((ConsRefState*)cons)->car;
    ((ConsRefState*)cons)->car = obj;
    Retain(obj);
    Release(prvObj);
}

Object ConsCdr(ConsRef cons) {
    _abortIfMismatch(cons, ConsTypeIdentifier);
    
    return cons ? ((ConsRefState*)cons)->cdr : nil;
}

void ConsSetCdr(ConsRef cons, Object obj) {
    Object prvObj = ((ConsRefState*)cons)->cdr;
    ((ConsRefState*)cons)->cdr = obj;
    Retain(obj);
    Release(prvObj);
}


ConsRef ConsCopy(ConsRef cons) {
    if (_Kind(cons) == ConsTypeIdentifier) {
        ConsRef consToCopyTo = cons ? ConsCreate(nil, nil) : nil;
        
        if (!consToCopyTo) {
            return nil;
        }
        
        Object carToMoveOver = ConsCar(cons);
        Object cdrToCopy = ConsCdr(cons);
        
        if (_Kind(cdrToCopy) != ConsTypeIdentifier && cdrToCopy) {
            abort();
        }
        
        ConsSetCar(consToCopyTo, carToMoveOver);
        ConsRef cdrCopy = ConsCopy(cdrToCopy);
        ConsSetCdr(consToCopyTo, cdrCopy);
        Release(cdrCopy);
        
        return consToCopyTo;
    } else {

        if (cons) {
            abort();
        }
        
        return cons;
    }
}

ConsRef ConsPush(ConsRef cons, Object obj) {
    return _ConsPush(cons, obj, YES);
}

ConsRef _ConsPush(ConsRef cons, Object obj, BOOL shouldAutoRelease) {
    ConsRef consCopy = ConsCopy(cons);
    
    ConsRef consOnTop = ConsCreate(obj, consCopy);
    
    Release(consCopy);
    
    if (shouldAutoRelease) {
        AutoRelease(consOnTop);
    }
    
    return consOnTop;
}

ConsRef ConsPop(ConsRef cons, Object *obj) {
    return _ConsPop(cons, obj, YES);
}

ConsRef _ConsPop(ConsRef cons, Object *obj, BOOL shouldAutoRelease) {
    if (cons) {
        Object objectToReturn = ConsCar(cons);
        ConsRef consCopy = ConsCopy(ConsCdr(cons));
        
        if (shouldAutoRelease) {
            AutoRelease(consCopy);
        }
        
        *obj = objectToReturn;
        
        return consCopy;
    }
    return nil;
}

ConsRef ConsAddToEnd(ConsRef cons, Object obj) {
    ConsRef consToReturn = nil;
    if (cons == nil) {
        consToReturn = AutoRelease(ConsCreate(obj, nil));
    } else if (!ConsCdr(cons)){
        ConsRef newCarCons = ConsCreate(obj, nil);
        ConsSetCdr(cons, newCarCons);
        Release(newCarCons);
        consToReturn = cons;
    } else {
        ConsRef cdr = ConsCdr(cons);
        ConsAddToEnd(cdr, obj);
        consToReturn = cons;
    }
    
    return consToReturn;
}

int ConsLength(ConsRef cons) {
    return cons ? 1 + ConsLength(ConsCdr(cons)) : 0;
}


#pragma mark Char

StringRef _CharDescription (Object obj) {
    CharRefState *state = (CharRefState*)obj;
    char *stringRep = calloc(1, 2);
    sprintf(stringRep, "%c", state->character);
    StringRef stringToReturn = StringCreate(stringRep);
    free(stringRep);
    return AutoRelease(stringToReturn);
}

void _CharDealloc(Object obj) {
    /* Nothing to do... */
}

CharRef CharCreate(char character) {
    CharRefState *newChar = _ObjectInitialize(CharTypeIdentifier, &_CharDealloc, &_CharDescription);
    newChar->character = character;
    return newChar;
}

char CharCChar(CharRef obj) {
    CharRefState *charRef = (CharRefState*)obj;

    if (charRef) {
        return charRef->character;
    } else {
        return -1;
    }
}

#pragma mark String

StringRef _StringDescription (Object obj) {
    StringRefState *string = (StringRefState*)obj;
    char *stringCString = StringCString(string);
    StringRef stringToReturn = AutoRelease(StringCreate(stringCString));
    free(stringCString);
    return stringToReturn;
}

void _StringDealloc(Object obj) {
    StringRefState *string = (StringRefState*)obj;
    Release(string->consRep);
    string->consRep = nil;
}

StringRef StringCreate(char *string) {
    StringRefState *newString = _ObjectInitialize(StringTypeIdentifier, &_StringDealloc, &_StringDescription);
    newString->consRep = nil;

    if (string) {
        int i = 0;
        do {
            CharRef *translatedChar = CharCreate(string[i]);
            newString->consRep = ConsAddToEnd(newString->consRep, translatedChar);
            Release(translatedChar);
            i++;
        } while (string[i] != '\0');
    }

    Retain(newString->consRep);

    return newString;
}

char *StringCString(StringRef obj) {
  int i = 0;
    if (!obj) {
        return NULL;
    }
    
    StringRefState *string = (StringRefState*)obj;
    unsigned int length = StringLength(string);
    char *cString = calloc(1, length + 1);
    ConsRef *stringCons = string->consRep;
    for (i = 0; i < length; i++) {
        cString[i] = CharCChar(((CharRef*)ConsCar(stringCons)));
        stringCons = ConsCdr(stringCons);
    }
    cString[length] = '\0';
    return cString;
}

void _StringSetConsRep(StringRefState *state, ConsRef consRep) {
    Release(state->consRep);
    state->consRep = consRep;
    Retain(state->consRep);
}

unsigned int StringLength(StringRef obj) {
    StringRefState *string = (StringRefState*)obj;
    return ConsLength(string->consRep);
}

BOOL StringEqual(StringRef stringZero, StringRef stringOne) {
  int i = 0;

    StringRefState *stringZeroState = (StringRefState*)stringZero;
    StringRefState *stringOneState = (StringRefState*)stringOne;

    unsigned int lengthZero = StringLength(stringZero);
    unsigned int lengthOne = StringLength(stringOne);

    if (lengthOne != lengthZero) {
        return NO;
    }

    ConsRef *stringConsZero = stringZeroState->consRep;
    ConsRef *stringConsOne = stringOneState->consRep;
    for (i = 0; i < lengthZero; i++) {
        char charZero = CharCChar(((CharRef*)ConsCar(stringConsZero)));
        char charOne = CharCChar(((CharRef*)ConsCar(stringConsOne)));

        if (charZero != charOne) {
            return NO;
        }

        stringConsZero = ConsCdr(stringConsZero);
        stringConsOne = ConsCdr(stringConsOne);
    }

    return YES;
}

StringRef StringConcatenate(StringRef string, StringRef stringToAdd) {
    char *origString = StringCString(string);
    char *newString = StringCString(stringToAdd);
    StringRef stringToReturn = StringCreate(origString);
    StringRefState *stringToReturnState = (StringRefState*)stringToReturn;
    
        int i = 0;
        do {
            CharRef *translatedChar = CharCreate(newString[i]);
            stringToReturnState->consRep = ConsAddToEnd(stringToReturnState->consRep, translatedChar);
            Release(translatedChar);
            i++;
        } while (newString[i] != '\0');
    
    free(origString);
    free(newString);
    
    return AutoRelease(stringToReturn);
}

void StringPrint(Object obj, const char *format) {
    StringRef objDescription = Description(obj);
    char *cRep = StringCString(objDescription);
    printf(format, cRep);
    free(cRep);
}

StringRef StringSPrint(Object obj, const char *format) {
    StringRef objDescription = Description(obj);
    char *cRep = StringCString(objDescription);
    char *sprint = calloc(1, strlen(cRep) + strlen(format));
    sprintf(sprint, format, cRep);
    StringRef toReturn = AutoRelease(StringCreate(sprint));
    free(cRep);
    free(sprint);
    return toReturn;
}
