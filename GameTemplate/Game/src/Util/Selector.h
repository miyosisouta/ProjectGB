#pragma once


template<typename T>
class Selector
{
public:
	Selector(T minVal, T maxVal, T step = T(1), T initial = T(0),
		EnButton decBtn = enButtonUp, EnButton incBtn = enButtonDown)
		: m_value(initial), m_min(minVal), m_max(maxVal), m_step(step),
		m_decBtn(decBtn), m_incBtn(incBtn), m_changed(false) {
	}

	void Update(int padIndex = 0) {
		T prev = m_value;
		if (g_pad[padIndex]->IsTrigger(m_decBtn)) {
			m_value -= m_step;
			if (m_value < m_min) m_value = m_min;
		}
		else if (g_pad[padIndex]->IsTrigger(m_incBtn)) {
			m_value += m_step;
			if (m_value > m_max) m_value = m_max;
		}
		m_changed = !IsEqual(m_value, prev);
	}

	T GetValue() const { return m_value; }
	bool IsChanged() const { return m_changed; }

	void SetValue(T v) {
		if (v < m_min) v = m_min;
		if (v > m_max) v = m_max;
		m_value = v;
	}

private:
	T m_value;
	T m_min;
	T m_max;
	T m_step;
	EnButton m_decBtn;
	EnButton m_incBtn;
	bool m_changed;

	// int等の整数型
	template<typename U = T>
	typename std::enable_if<std::is_integral<U>::value, bool>::type
		IsEqual(U a, U b) const {
		return a == b;
	}

	// float/double
	template<typename U = T>
	typename std::enable_if<std::is_floating_point<U>::value, bool>::type
		IsEqual(U a, U b) const {
		return std::abs(a - b) < static_cast<U>(1e-6);
	}
};

using IntSelector = Selector<int>;
using FloatSelector = Selector<float>;