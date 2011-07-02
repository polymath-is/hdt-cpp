/*
 * RDFSerializerRaptor.cpp
 *
 *  Created on: 05/03/2011
 *      Author: mck
 */

#ifdef USE_RAPTOR
#include "RDFSerializerRaptor.hpp"

using namespace std;

namespace hdt {

RDFSerializerRaptor::RDFSerializerRaptor(std::ostream &s) : RDFSerializer(s) {

}

RDFSerializerRaptor::~RDFSerializerRaptor() {

}

int iostream_write_bytes (void *context, const void *ptr, size_t size, size_t nmemb) {
	std::ostream *out = reinterpret_cast<std::ostream *>(context);
	if(out->good()) {
		out->write((const char *)ptr, size*nmemb);
		return 0;
	} else {
		return 1;
	}
}

int iostream_write_byte(void *context, const int byte) {
	std::ostream *out = reinterpret_cast<std::ostream *>(context);
	if(out->good()) {
		out->write((const char *)&byte, sizeof(int));
		return 0;
	} else {
		return 1;
	}
}

raptor_term *getTerm(string &str, raptor_world *world) {
	if(str=="") {
		return NULL;
	}

	if(str.at(0)=='<'){
		return raptor_new_term_from_uri_string(world, (const unsigned char *)str.substr(1, str.length()-2).c_str());
	} else if(str.at(0)=='"') {
		return raptor_new_term_from_literal(world, (const unsigned char *)str.substr(1, str.length()-2).c_str(), NULL, NULL);
	} else if(str.at(0)=='_') {
		return raptor_new_term_from_blank(world, (const unsigned char *)str.c_str());
	} else {
		throw "Unknown kind of term";
	}
}

void RDFSerializerRaptor::serialize(IteratorTripleString *it)
{
	world = raptor_new_world();

	raptor_uri* uri = raptor_new_uri(world, (const unsigned char*)"http://www.rdfhdt.org/");

	raptor_iostream_handler handler;
	memset(&handler, 0, sizeof(handler));
	handler.write_byte = iostream_write_byte;
	handler.write_bytes = iostream_write_bytes;

	raptor_iostream *iostream = raptor_new_iostream_from_handler(world, (void *) &stream, &handler);
	raptor_serializer* rdf_serializer = raptor_new_serializer(world, "ntriples") ;

	raptor_serializer_start_to_iostream(rdf_serializer, uri, iostream);

	while( it->hasNext() ) {
		TripleString *ts = it->next();

		if(!ts->isEmpty()) {
			raptor_statement* triple = raptor_new_statement(world);
			triple->subject = getTerm(ts->getSubject(), world);
			triple->predicate = getTerm(ts->getPredicate(), world);
			triple->object = getTerm(ts->getObject(), world);
			raptor_serializer_serialize_statement(rdf_serializer, triple);
			raptor_free_statement(triple);
		}
	}
	raptor_serializer_serialize_end(rdf_serializer);

	raptor_free_serializer(rdf_serializer);

	raptor_free_iostream(iostream);

	raptor_free_world(world);
}

void RDFSerializerRaptor::endProcessing()
{
}

}
#endif