#include "XmlTokenWriter.h"

const char* XmlTokenWriter::get_error()
{
	return "STUB";
}

bool XmlTokenWriter::check_string_set_error(const char* str)
{
	if (!str)
	{
		error = ERROR_NULL_PTR;
		return false;
	}

	size_t len = strlen(str);
	if (len == 0)
	{
		error = ERROR_ZERO_STRING;
		return false;
	}
	
	error = ERROR_NONE;
	return true;

}

// <
void XmlTokenWriter::write_left_simple_bracket(Stream& s)
{
	s.print("<");
}
// </
void XmlTokenWriter::write_left_closing_bracket(Stream& s)
{
	s.print("</");
}

// <tagname + space
void XmlTokenWriter::write_tag_opening(Stream& s, const char* str)
{
	if (!check_string_set_error(str))
	{
		return;
	}
	write_left_closing_bracket(s);
	s.print(str);
	s.print(" ");
}
// > + newline
void XmlTokenWriter::write_right_simple_bracket(Stream& s)
{
	s.println(">");
}

// /> + newline
void XmlTokenWriter::write_right_closing_bracket(Stream& s)
{
	s.println("/>");
}

// </tagname>
void XmlTokenWriter::write_tag_closing(Stream& s, const char *str)
{
	if (!check_string_set_error(str))
	{
		return;
	}
	write_left_closing_bracket(s);
	s.print(str);
	write_right_simple_bracket(s);
}

// attr_name="attr_value" + space
void XmlTokenWriter::write_attribute_text(Stream& s, const char* attr_name, const char *attr_value)
{
	if (!check_string_set_error(attr_name))
	{
		return;
	}
	if (!check_string_set_error(attr_value))
	{
		return;
	}

	s.print(attr_name);
	s.print("=\"");
	s.print(attr_value);
	s.print("\"");
	s.print(" ");
}
void XmlTokenWriter::write_attribute_num(Stream& s, const char* attr_name, int n)
{
	if (!check_string_set_error(attr_name))
	{
		return;
	}

	const size_t BUFLEN = 20;
	char buf[BUFLEN];
	snprintf(buf, BUFLEN, "%d", n);
	buf[sizeof(BUFLEN) - 1] = '\0';
	

	s.print(attr_name);
	s.print("=\"");
	s.print(buf);
	s.print("\"");
	s.print(" ");
}
