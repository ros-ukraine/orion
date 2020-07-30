/*
 * orion_minor.h
 *
 *  Created on: 16 лип. 2020 р.
 *      Author: Bogdan
 */

#ifndef THIRD_PARTY_LIBS_ORION_ORION_MINOR_H_
#define THIRD_PARTY_LIBS_ORION_ORION_MINOR_H_

namespace orion {

class Minor {
public:
	Minor();
	virtual ~Minor();

	void receiveCommand(void);
};

} /* namespace orion */

#endif /* THIRD_PARTY_LIBS_ORION_ORION_MINOR_H_ */
