////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// TimeUnit.h - header for implementation of the atomic unit of time that the
// EDF algorithm is scheduling a task for.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <ctime> //for ticksRemaining
#include <chrono> //for all other time related functionality
#define NS_PER_TICK 10

using namespace std::chrono; //for all other time related functionality

/**
 * @brief Implements the atmoic unit of time that the EDF algorithm is
 * scheduling a task for.
*/
class TimeUnit
{
public:
	/**
	 * @brief The time, in nanoseconds, at which the TimeUnit starts.
	*/
	time_point <system_clock, nanoseconds> startTime;

	/**
	 * @brief The time, in nanoseconds, at which the TimeUnit ends.
	*/
	time_point <system_clock, nanoseconds> endTime;

	/**
	 * @brief The unique identifier for the time unit. Sequential, with 0 at
	 *		  program start.
	*/
	const unsigned int unitNum;

	/**
	 * @brief Constructor for the TimeUnit. Initializes the unitNum, as well as
	 * numClocks.
	 * @param unitNum   - The unique identifier for the time unit. Sequential, 
	 *					  with 0 at program start.
	 * @param numClocks - The number of clock cycles in the TimeUnit.
	*/
	TimeUnit(const unsigned int unitNum, const unsigned int numClocks);

	/**
	 * @brief Destructor for the TimeUnit instance.
	*/
	~TimeUnit();

	/**
	 * @brief Starts the TimeUnit. Sets the startTime and endTime.
	*/
	void start();

	/**
	 * @brief Calculates the number of clock cycles remaining in the TimeUnit.
	 * @return The number of cycles remaining, or 0 if the TimeUnit has ended.
	*/
	clock_t ticksRemaining();

	/**
	 * @brief Calculates the number of nanoseconds remaining in the TimeUnit.
	 * @return The time remaining (in nanoseconds), or 0 if the TimeUnit has
	 *		   ended.
	*/
	nanoseconds timeRemaining();

	/**
	 * @brief Subtracts the unitIDs of two units (unit1 - unit2).
	 * @param unit1 - The unit to be subtracted from.
	 * @param unit2 - The unit to be subtracted.
	 * @return		  difference in unitIds.
	*/
	static unsigned int unitDiff(const TimeUnit& unit1, const TimeUnit& unit2);

	/**
	 * @brief Overrides the ">" operator to compare unitIds.
	 * @param unit1 - The left hand side of the comparison.
	 * @param unit2 - The right hand side of the compasion.
	 * @return		  True if the unitID for unit1 is greater, False otherwise.
	*/
	friend bool operator> (const TimeUnit& unit1, const TimeUnit& unit2);

	/**
	 * @brief Overrides the "<" operator to compare unitIds.
	 * @param unit1 - The left hand side of the comparison.
	 * @param unit2 - The right hand side of the compasion.
	 * @return		  True if the unitID for unit1 is lesser, False otherwise.
	*/
	friend bool operator< (const TimeUnit& unit1, const TimeUnit& unit2);

	/**
	 * @brief Overrides the ">=" operator to compare unitIds.
	 * @param unit1 - The left hand side of the comparison.
	 * @param unit2 - The right hand side of the compasion.
	 * @return		  True if the unitID for unit1 is greater than or equal to 
	 *				  that of unit2, False otherwise.
	*/
	friend bool operator>= (const TimeUnit& unit1, const TimeUnit& unit2);

	/**
	 * @brief Overrides the "<=" operator to compare unitIds.
	 * @param unit1 - The left hand side of the comparison.
	 * @param unit2 - The right hand side of the compasion.
	 * @return		  True if the unitID for unit1 is less than or equal to
	 *				  that of unit2, False otherwise.
	*/
	friend bool operator<= (const TimeUnit& unit1, const TimeUnit& unit2);

	/**
	 * @brief Overrides the "==" operator to compare unitIds.
	 * @param unit1 - The left hand side of the comparison.
	 * @param unit2 - The right hand side of the compasion.
	 * @return		  True if the unitID for unit1 is equal to that of unit2, 
	 *				  False otherwise.
	*/
	friend bool operator== (const TimeUnit& unit1, const TimeUnit& unit2);

	/**
	 * @brief Overrides the "!=" operator to compare unitIds.
	 * @param unit1 - The left hand side of the comparison.
	 * @param unit2 - The right hand side of the compasion.
	 * @return		  True if the unitID for unit1 is not equal to that of 
	 *				  unit2, False otherwise.
	*/
	friend bool operator!= (const TimeUnit& unit1, const TimeUnit& unit2);

private:

	/**
	 * @brief The number of clock cycles in the TimeUnit.
	*/
	const unsigned int numClocks;
}; //TimeUnit
