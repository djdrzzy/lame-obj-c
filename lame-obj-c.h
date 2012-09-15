//
//  lame-obj-c.h
//  lame-obj-c
//
//  Created by Daniel Drzimotta on 2012-09-05.
//  Copyright (c) 2012 Daniel Drzimotta. All rights reserved.
//

#pragma mark Base Object System

#define YES 1
#define NO 0

#define BOOL int
#define nil NULL

static const unsigned int kTypesOfObjectsAllowed = 5000;

typedef unsigned int ObjectType;

typedef unsigned int RefCount;

typedef void * Object;
typedef Object ConsRef;
typedef Object AutoReleasePoolRef;
typedef Object CharRef;
typedef Object StringRef;

typedef void(*DeallocFunc)(Object obj);
typedef StringRef(*DescriptionFunc)(Object obj);

void SetupObjectSystem();
void RegisterObjectType(ObjectType type, ssize_t objectSize);
ssize_t RegisteredObjectSize(ObjectType type);

void Retain(Object obj);
RefCount RetainCount(Object obj);
void Release(Object obj);
Object AutoRelease(Object obj);
void AutoReleasePoolCreate();
void AutoReleasePoolDrain();

StringRef Description(Object obj);

ConsRef ConsCreate(Object car, Object cdr);
Object ConsCar(ConsRef cons);
void ConsSetCar(ConsRef cons, Object obj);
Object ConsCdr(ConsRef cons);
void ConsSetCdr(ConsRef cons, Object obj);

ConsRef ConsPush(ConsRef cons, Object obj);
ConsRef ConsPop(ConsRef cons, Object *obj);
ConsRef ConsAddToEnd(ConsRef cons, Object obj);
ConsRef ConsCopy(ConsRef cons);

int ConsLength(ConsRef cons);

unsigned int numberOfConsCreated();
unsigned int numberOfLeakedCons();


CharRef CharCreate(char character);
char CharCChar(CharRef character);

// It will stop copying the string over if '\0' is found.
StringRef StringCreate(char *string);
// You are responsible for freeing the return val
char * StringCString(StringRef string);

unsigned int StringLength(StringRef string);
BOOL StringEqual(StringRef stringZero, StringRef stringOne);

// Returns a new string with the 2 strings concatenated together.
StringRef StringConcatenate(StringRef string, StringRef stringToAdd);


// Expects a format string with 1 "%s" in it for where the Description of the object should print
void StringPrint(Object obj, const char *format);

// Same as above but returns a string rather than printing it
StringRef StringSPrint(Object obj, const char *format);

