#include <catch2/catch.hpp>

#include <iostream>
#include <limits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

#include <test/helper.hpp>

using namespace json_dto;

template < typename T >
bool
eq( const T & left, const T & right )
{
	return left == right;
}

template < typename T >
bool
eq( const nullable_t< T > & left, const nullable_t< T > & right )
{
	if( !left && !right )
		return true;

	return left && right && eq( *left, *right );
}

//
// custom_reader_writer_t
//
struct custom_reader_writer_t
{
	template< typename Field_Type >
	void
	read( Field_Type & v, const rapidjson::Value & from ) const
	{
		using json_dto::read_json_value;
		read_json_value( v, from );
	}

	template< typename Field_Type >
	void
	write(
		Field_Type & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		write_json_value( v, to, allocator );
	}
};

//
// simple_types_dto_t
//

struct simple_types_dto_t
{
	std::int32_t m_num{ -1 };
	std::int32_t m_num_opt{ -1 };
	std::int32_t m_num_opt_no_default{ -1 };

	nullable_t< std::int32_t > m_num_opt_nullable{};
	nullable_t< std::int32_t > m_num_opt_no_default_nullable{};

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory( custom_reader_writer_t{}, "num", m_num )
			& optional(
					custom_reader_writer_t{},
					"num_opt", m_num_opt, 0 )
			& optional_no_default(
					custom_reader_writer_t{},
					"num_opt_no_default", m_num_opt_no_default )
			& optional(
					custom_reader_writer_t{},
					"num_opt_nullable", m_num_opt_nullable, nullptr )
			& optional_no_default(
					custom_reader_writer_t{},
					"num_opt_no_default_nullable", m_num_opt_no_default_nullable )
		;
	}

	bool
	operator == ( const simple_types_dto_t & other ) const
	{
		return
			eq( m_num, other.m_num ) &&
			eq( m_num_opt, other.m_num_opt ) &&
			eq( m_num_opt_no_default, other.m_num_opt_no_default ) &&
			eq( m_num_opt_nullable, other.m_num_opt_nullable ) &&
			eq( m_num_opt_no_default_nullable, other.m_num_opt_no_default_nullable );
	}
};

TEST_CASE("simple-types", "[simple]" )
{
	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >(
				R"({"num":1})" );

		REQUIRE( dto.m_num == 1 );
		REQUIRE( dto.m_num_opt == 0 );
		REQUIRE( dto.m_num_opt_no_default == -1 );
		REQUIRE_FALSE( dto.m_num_opt_nullable );
		REQUIRE_FALSE( dto.m_num_opt_no_default_nullable );
	}

	SECTION( "write default constructed" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num":-1,
					"num_opt":-1,
					"num_opt_no_default":-1,
					"num_opt_no_default_nullable":null
				})JSON" );

		simple_types_dto_t dto{};

		REQUIRE( json_str == to_json( dto ) );
	}

	SECTION( "write default values" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num":-1,
					"num_opt_no_default":-1,
					"num_opt_no_default_nullable":null
				})JSON" );

		simple_types_dto_t dto{};
		dto.m_num_opt = 0;

		REQUIRE( json_str == to_json( dto ) );
	}

	const std::string all_defined_json =
		zip_json_str(
			R"JSON({
				"num":333,
				"num_opt":42,
				"num_opt_no_default":-99999,
				"num_opt_nullable":999,
				"num_opt_no_default_nullable":2016
			})JSON" );

	SECTION( "read all defined" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >( all_defined_json );

		REQUIRE( dto.m_num == 333 );
		REQUIRE( dto.m_num_opt == 42 );
		REQUIRE( dto.m_num_opt_no_default == -99999 );
		REQUIRE( dto.m_num_opt_nullable );
		REQUIRE( *dto.m_num_opt_nullable == 999 );
		REQUIRE( dto.m_num_opt_no_default_nullable );
		REQUIRE( *dto.m_num_opt_no_default_nullable == 2016 );
	}

	SECTION( "write default values" )
	{
		simple_types_dto_t dto{};
		dto.m_num = 333;
		dto.m_num_opt = 42;
		dto.m_num_opt_no_default = -99999;
		dto.m_num_opt_nullable.emplace( 999 );
		dto.m_num_opt_no_default_nullable.emplace( 2016 );

		REQUIRE( all_defined_json == to_json( dto ) );
	}
}

