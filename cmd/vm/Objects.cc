#include <cassert>
#include <iostream>

#include "Misc.hh"
#include "ObjectMemory.hh"
#include "Oops.hh"

/**
 * \defgroup DictionaryOop
 * @{
 */

void
DictionaryOopDesc::insert(intptr_t hash, Oop key, Oop value)
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
			nwLink = LinkOopDesc::newWith(key, value);
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

ClassOop
DictionaryOopDesc::findOrCreateClass(ObjectMemory &omem, ClassOop superClass,
    std::string name)
{
	ClassOop result = symbolLookup(name).as<ClassOop>();

	if (result.isNil()) {
		printf("Nil - allocating new class %s\n", name.c_str());
		result = ClassOopDesc::allocate(omem, superClass, name);
	} else
		result->setupClass(superClass, name);

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
	Oop res = symbolLookup(first);

	printf("Lookup %s/%s\n", first.c_str(),
	    ind != std::string::npos ? name.substr(ind + 1).c_str() : "");

	if (res.isNil()) {
		DictionaryOop super = symbolLookup("Super").as<DictionaryOop>();
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
	dict->basicAtPut(1, ArrayOopDesc::newWithSize(numBuckets * 3));
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
		/*key->print (in + 2);
		std::cout << blanks (in + 1) + " Value:\n";
		if (!(key.isa () == memMgr.clsSymbol &&
		      key.as<SymbolOop> ()->strEquals ("Super")))
		    value->print (in + 2);
		else
		    std::cout << blanks (in + 2) << "<Super-entry>\n";
		    */

		std::cout << blanks(in + 1) + "}\n";

		for (link = *(LinkOop *)hp; !link.isNil();
		     link = *(LinkOop *)hp) {
			hp = (Oop *)link->vns();
			key = *hp++;   /* link at: 1 */
			value = *hp++; /* link at: 2 */
			std::cout << blanks(in + 1) + "{ Key:\n";
			// key->print (in + 2);
			std::cout << blanks(in + 1) + " Value:\n";
			// value->print (in + 2);
			std::cout << blanks(in + 1) + "}\n";
		}
	}

	std::cout << blanks(in) + "}\n";
}

/**
 * @}
 */
