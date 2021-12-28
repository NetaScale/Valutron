#include <cassert>
#include <iostream>
#include <string.h>

#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"

template <class T>
void OopRef<T>::print(size_t in)
{
    if (isNil ())
        std::cout << blanks (in) + "<nil>\n";
    else if (*this == ObjectMemory::objFalse)
        std::cout << blanks (in) + "<FALSE>\n";
    else if (*this == ObjectMemory::objTrue)
        std::cout << blanks (in) + "<TRUE>\n";
    else if (isa () == ObjectMemory::clsDictionary ||
             isa () == ObjectMemory::clsSystemDictionary)
        as<DictionaryOop> ()->print (in);
#define Case(ClassName)                                                        \
    else if (isa () == ObjectMemory::cls##ClassName)                    \
        as<ClassName##Oop> ()->print (in)
    else if (isa () == ObjectMemory::clsInteger)
        std::cout << blanks(in) + "<SMI: " << m_smi << ">\n";
    else if (isa () == ObjectMemory::clsString)
        as<SymbolOop> ()->print (in);
    Case (Symbol);
//    Case (Method);
//    Case (Block);
    Case (Link);
    /*else if ((anOop->index () > ObjectMemory::clsObject ().index ()) &&
             (anOop->index () < ObjectMemory::clsSystemDictionary ().index ()))
        anOop->asClassOop ()
            ->print (in);*/
    else std::cout << blanks (in) + "Unknown object: ptr " << m_ptr
                   << " .\n";
}

int
strHash(std::string str)
{
	int32_t hash;
	const char *p;

	hash = 0;
	for (p = str.c_str(); *p; p++)
		hash += *p;
	if (hash < 0)
		hash = -hash;
	/* scale to proper size */
	while (hash > (1 << 24))
		hash >>= 2;
	return hash;
}

ArrayOop
ArrayOopDesc::newWithSize(ObjectMemory &omem, size_t size)
{
	ArrayOop newArr = omem.newOopObj<ArrayOop>(size);
	return newArr;
}

ArrayOop
ArrayOopDesc::fromVector(ObjectMemory &omem, std::vector<Oop> vec)
{
	ArrayOop newArr = newWithSize(omem, vec.size());
	std::copy(vec.begin(), vec.end(), newArr->vns());
	return newArr;
}

ArrayOop
ArrayOopDesc::symbolArrayFromStringVector(ObjectMemory &omem,
    std::vector<std::string> vec)
{
	std::vector<Oop> symVec;

	for (auto s : vec)
		symVec.push_back(SymbolOopDesc::fromString(omem, s));

	return fromVector(omem, symVec);
}

ByteArrayOop
ByteArrayOopDesc::newWithSize(ObjectMemory &omem, size_t size)
{
	ByteArrayOop newArr = omem.newByteObj<ByteArrayOop>(size);
	newArr.setIsa(ObjectMemory::clsByteArray);
	return newArr;
}

ByteArrayOop
ByteArrayOopDesc::fromVector(ObjectMemory &omem, std::vector<uint8_t> vec)
{
	ByteArrayOop newArr = newWithSize(omem, vec.size());
	std::copy(vec.begin(), vec.end(), newArr->vns());
	return newArr;
}

CharOop
CharOopDesc::newWith(ObjectMemory &omem, intptr_t value)
{
	CharOop newChar = omem.newOopObj<CharOop>(1);
	newChar.setIsa(ObjectMemory::clsChar);
	newChar->setValue(value);
	return newChar;
}

LinkOop
LinkOopDesc::newWith(ObjectMemory &omem, Oop a, Oop b)
{
	LinkOop newLink = omem.newOopObj<LinkOop>(3);
	newLink.setIsa(ObjectMemory::clsLink);
	newLink->setOne(a);
	newLink->setTwo(b);
	return newLink;
}

void LinkOopDesc::print (int in)
{
    std::cout << blanks (in) << "Link " << this << "{\n";
    std::cout << blanks (in) << "One:\n";
    one ().print (in + 2);
    std::cout << blanks (in) << "Two:\n";
    two ().print (in + 2);
    std::cout << blanks (in) << "nextLink:\n";
    nextLink ()->print (in + 2);
    std::cout << blanks (in) << "}\n";
}


/**
 * \defgroup ClassOop
 * @{
 */

void
ClassOopDesc::addMethod(ObjectMemory &omem, MethodOop method)
{
	if (methods().isNil())
		setMethods(DictionaryOopDesc::newWithSize(omem, 20));
	methods()->symbolInsert(omem, method->selector(), method);
	method->setMethodClass(this);
}

void
ClassOopDesc::addClassMethod(ObjectMemory &omem, MethodOop method)
{
	isa()->addMethod(omem, method);
}

void
ClassOopDesc::setupClass(ObjectMemory &omem, ClassOop superClass,
    std::string name)
{
	/* allocate metaclass if necessary */
	if (isa().isNil())
		setIsa(ClassOopDesc::allocateRawClass(omem));

	/* set metaclass isa to object metaclass */
	isa().setIsa(ObjectMemory::clsObjectMeta);

	isa()->setName(SymbolOopDesc::fromString(omem, name + "Meta"));
	setName(SymbolOopDesc::fromString(omem, name));

	setupSuperclass(superClass);
}

void
ClassOopDesc::setupSuperclass(ClassOop superClass)
{
	if (!superClass.isNil()) {
		isa()->setSuperClass(superClass.isa());
		setSuperClass(superClass);
	} else /* root object */
	{
		isa()->setSuperClass(this);
		/* class of Object is the terminal class for both the metaclass
		 * and class hierarchy */
	}
}

ClassOop
ClassOopDesc::allocateRawClass(ObjectMemory &omem)
{
	ClassOop cls = omem.newOopObj<ClassOop>(clsInstLength);
	return cls;
}

ClassOop
ClassOopDesc::allocate(ObjectMemory &omem, ClassOop superClass,
    std::string name)
{
	ClassOop metaCls = omem.newOopObj<ClassOop>(clsInstLength),
		 cls = omem.newOopObj<ClassOop>(clsInstLength);

	cls->setIsa(metaCls);
	cls->setupClass(omem, superClass, name);

	return cls;
}

ClassPair
ClassPair::allocateRaw(ObjectMemory &omem)
{
	ClassOop metaCls = ClassOopDesc::allocateRawClass(omem);
	ClassOop cls = ClassOopDesc::allocateRawClass(omem);
	return ClassPair(cls, metaCls);
}

/**
 * @}
 */

/**
 * \defgroup DictionaryOop
 * @{
 */

int
hashCmp(Oop key, Oop match)
{
	/*  DEBUG  printf ("Compare %s to %s\n",
		   key->asStringOop ()->asCStr (),
		   match->asStringOop ()->asCStr ());*/
	return key.hashCode() == match.hashCode();
}

void
DictionaryOopDesc::insert(ObjectMemory &omem, intptr_t hash, Oop key, Oop value)
{
	ArrayOop table;
	LinkOop link, nwLink, nextLink;
	Oop tablentry;

	/* first get the hash table */
	table = basicAt(0).as<ArrayOop>();

	if (table->size() < 3) {
		perror("attempt to insert into too small name table");
	} else {
		hash = 3 * (hash % (table->size() / 3));
		assert(hash <= table->size() - 3);
		tablentry = table->basicAt(hash);

		/* FIXME: I adapted this from the PDST C sources, and this
		 * doesn't appear to handle anything other than symbols (because
		 * of tablentry == key). Make it a template in the future or
		 * make it accept a comparison callback like findPairByFun() in
		 * the future, maybe?
		 */
		if (tablentry.isNil() || (tablentry == key)) {
			table->basicAtPut(hash, key);
			table->basicAtPut(hash + 1, value);
		} else {
			nwLink = LinkOopDesc::newWith(omem, key, value);
			link = table->basicAt(hash + 2).as<LinkOop>();
			if (link.isNil()) {
				table->basicAtPut(hash + 2, nwLink);
			} else
				while (1)
					/* ptrEq (orefOf (link, 1),
					 * (objRef)key)) */
					if (link->one() == key) {
						/* get rid of unwanted Link */
						// isVolatilePut (nwLink,
						// false);
						link->setTwo(value);
						break;
					} else if ((nextLink = link->nextLink())
						       .isNil()) {
						link->setNextLink(nwLink);
						break;
					} else
						link = nextLink;
		}
	}
}

template <typename ExtraType>
std::pair<Oop, Oop>
DictionaryOopDesc::findPairByFun(intptr_t hash, ExtraType extraVal,
    int (*fun)(Oop, ExtraType))
{
	ArrayOop table = basicAt(0).as<ArrayOop>();
	Oop key, value;
	LinkOop link;
	Oop *hp;
	int tablesize;

	/* now see if table is valid */
	if ((tablesize = table->size()) < 3) {
		printf("Table Size: %d\n", tablesize);
		perror("system error lookup on null table");
	} else {
		hash = 1 + (3 * (hash % (tablesize / 3)));
		assert(hash <= tablesize - 2);
		hp = (Oop *)table->vns() + (hash - 1);
		key = *hp++;   /* table at: hash */
		value = *hp++; /* table at: hash + 1 */
		if ((!key.isNil()) && (*fun)(key, extraVal))
			return { key, value };
		for (link = *(LinkOop *)hp; !link.isNil();
		     link = *(LinkOop *)hp) {
			hp = (Oop *)link->vns();
			key = *hp++;   /* link at: 1 */
			value = *hp++; /* link at: 2 */
			if (!key.isNil() && (*fun)(key, extraVal))
				return { key, value };
		}
	}

	return { Oop(), Oop() };
}

ClassOop
DictionaryOopDesc::findOrCreateClass(ObjectMemory &omem, ClassOop superClass,
    std::string name)
{
	ClassOop result = symbolLookup(omem, name).as<ClassOop>();

	if (result.isNil()) {
		printf("Nil - allocating new class %s\n", name.c_str());
		result = ClassOopDesc::allocate(omem, superClass, name);
	} else
		result->setupClass(omem, superClass, name);

	return result;
}

DictionaryOop
DictionaryOopDesc::subNamespaceNamed(ObjectMemory &omem, std::string name)
{
	size_t ind = name.find(':');
	if (ind == std::string::npos) {
		SymbolOop sym = SymbolOopDesc::fromString(omem, name);
		/* no more colons nested */
		DictionaryOop ns = symbolLookup(sym).as<DictionaryOop>();
		if (ns.isNil()) {
			printf("no such namespace %s, creating\n",
			    name.c_str());
			ns = DictionaryOopDesc::newWithSize(omem, 5);
			symbolInsert(omem, sym, ns);
			ns->symbolInsert(omem,
			    SymbolOopDesc::fromString(omem, "Super"), this);
		}
		return ns;
	} else {
		throw std::runtime_error("Subnamespace problem");
	}
}

Oop
DictionaryOopDesc::symbolLookupNamespaced(ObjectMemory &omem, std::string name)
{
	size_t ind = name.find(':');
	std::string first = ind != std::string::npos ? name.substr(0, ind) :
							     name;
	SymbolOop sym = SymbolOopDesc::fromString(omem, name);
	Oop res = symbolLookup(omem, first);

	printf("Lookup %s/%s\n", first.c_str(),
	    ind != std::string::npos ? name.substr(ind + 1).c_str() : "");

	if (res.isNil()) {
		DictionaryOop super =
		    symbolLookup(omem, "Super").as<DictionaryOop>();
		if (!super.isNil())
			res = super->symbolLookupNamespaced(omem, name);
	}

	if (ind != std::string::npos && !res.isNil())
		return res.as<DictionaryOop>()->symbolLookupNamespaced(omem,
		    name.substr(ind + 1));
	return res;
}

DictionaryOop
DictionaryOopDesc::newWithSize(ObjectMemory &omem, size_t numBuckets)
{
	DictionaryOop dict = omem.newOopObj<DictionaryOop>(1);
	dict->setIsa(ObjectMemory::clsDictionary);
	dict->basicAtPut(0, ArrayOopDesc::newWithSize(omem, numBuckets * 3));
	return dict;
}

void
DictionaryOopDesc::print(int in)
{
	ArrayOop table = basicAt(0).as<ArrayOop>();
	Oop key, value;
	LinkOop link;
	Oop *hp;
	int tablesize;

	std::cout << blanks(in) + "Dictionary {\n";

	/* now see if table is valid */
	if ((tablesize = table->size()) < 3) {
		printf("In Print: Table Size: %d\n", tablesize);
		perror("system error lookup on null table");
	}
	for (int i = 1; i <= table->size(); i += 3) {
		hp = (Oop *)table->vns() + (i - 1);
		// printf (" --> bucket: %d\n",
		//        i - 1); //(3 * (i % (tablesize / 3))) - 1);
		key = *hp++;   /* table at: hash */
		value = *hp++; /* table at: hash + 1 */

		std::cout << blanks(in + 1) + "{ Key:\n";
		key.print (in + 2);
		std::cout << blanks (in + 1) + " Value:\n";
		if (!(key.isNil() || key.isa () == ObjectMemory::clsSymbol &&
		      key.as<SymbolOop> ()->strEquals ("Super")))
		    value.print (in + 2);
		else
		    std::cout << blanks (in + 2) << "<Super-entry>\n";


		std::cout << blanks(in + 1) + "}\n";

		for (link = *(LinkOop *)hp; !link.isNil();
		     link = *(LinkOop *)hp) {
			hp = (Oop *)link->vns();
			key = *hp++;   /* link at: 1 */
			value = *hp++; /* link at: 2 */
			std::cout << blanks(in + 1) + "{ Key:\n";
			key.print (in + 2);
			std::cout << blanks(in + 1) + " Value:\n";
			value.print (in + 2);
			std::cout << blanks(in + 1) + "}\n";
		}
	}

	std::cout << blanks(in) + "}\n";
}

void
DictionaryOopDesc::symbolInsert(ObjectMemory &omem, SymbolOop key, Oop value)
{
	insert(omem, key.hashCode(), key, value);
}

Oop
DictionaryOopDesc::symbolLookup(SymbolOop aSymbol)
{
	return findPairByFun<Oop>(aSymbol->hashCode(), aSymbol, hashCmp).second;
}

Oop
DictionaryOopDesc::symbolLookup(ObjectMemory &omem, std::string aString)
{
	SymbolOop sym = SymbolOopDesc::fromString(omem, aString);
	return symbolLookup(sym);
}

/**
 * @}
 */

int
strTest(Oop key, std::string aString)
{
	if (key.as<StringOop>()->strEquals(aString))
		return 1;
	return 0;
}

SymbolOop
SymbolOopDesc::fromString(ObjectMemory &omem, std::string aString)
{
	SymbolOop newObj = ObjectMemory::objSymbolTable->
	    findPairByFun(strHash(aString), aString,strTest).first.
	    as<SymbolOop>();

	if (!newObj.isNil())
		return newObj;

	/* not found, must make */
	newObj = omem.newByteObj<SymbolOop>(aString.size() + 1);

	newObj.setIsa(ObjectMemory::clsSymbol);
	strncpy((char *)newObj->vns(), aString.c_str(), aString.size());
	ObjectMemory::objSymbolTable->insert(omem, strHash(aString.c_str()),
	    newObj, Oop());

	return newObj;
}

void SymbolOopDesc::print (int in)
{
    std::cout << blanks (in) << "Symbol " << this << ": "
              << (const char *)vns () << "\n";
}