#pragma once

#include <cstdint>

#include <QString>
#include <QTextStream>

/// Standard pointer size of source machine, in bits
#define STD_SIZE    32

/* pointer. size depends on platform */
class Address
{
public:
	typedef uintptr_t   value_type;

public:
	static const Address ZERO;
	static const Address INVALID;

	explicit Address() : m_value(0) {}
	explicit Address(value_type value) : m_value(value) {}

	Address(const Address&) = default;
	Address& operator=(const Address&) = default;

	static Address g(value_type x);   // construct host/native oblivious address
	static Address n(value_type x);   // construct native address

	static Address host_ptr(const void *x)
	{
		return Address((value_type)x);
	}

	Address        native() const { return Address::g(m_value & 0xFFFFFFFF); }
	value_type     value() const { return m_value; }

	bool           isZero() const { return m_value == 0; }
	bool operator==(const Address& other) const { return m_value == other.value(); }
	bool operator!=(const Address& other) const { return m_value != other.value(); }
	bool operator<(const Address& other) const { return m_value < other.value(); }
	bool operator>(const Address& other) const { return m_value > other.value(); }
	bool operator>=(const Address& other) const { return m_value >= other.value(); }
	bool operator<=(const Address& other) const { return m_value <= other.value(); }

	Address operator+(const Address& other) const { return Address::g(m_value + other.value()); }
	Address operator-(const Address& other) const { return Address::g(m_value - other.value()); }

	Address operator++() { ++m_value; return *this; }
	Address operator--() { --m_value; return *this; }
	Address operator++(int)	{ return Address(m_value++); }
	Address operator--(int) { return Address(m_value--); }

	Address operator+=(const Address& other) { m_value += other.value(); return *this; }

	Address operator+=(intptr_t other) { m_value += other; return *this; }

	Address operator+(intptr_t val) const { return Address::g(m_value + val); }

	Address operator-=(intptr_t v) { m_value -= v; return *this; }

	Address operator-(intptr_t other) const { return Address::g(m_value - other); }
	friend QTextStream& operator<<(QTextStream& os, const Address& mdv);

	QString        toString(bool zerofill = false) const
	{
		if (zerofill) {
			return "0x" + QString("%1").arg(m_value, 8, 16, QChar('0'));
		}

		return "0x" + QString::number(m_value, 16);
	}

private:
	value_type     m_value;
};

template<class T, class U>
bool IN_RANGE(const T& val, const U& range_start, const U& range_end)
{
	return((val >= range_start) && (val < range_end));
}

Q_DECLARE_METATYPE(Address)