//
// Created by kier on 2020/8/1.
//

#ifndef OMEGA_MACRO_H
#define OMEGA_MACRO_H

#define __ohm_concat_string__(x,y) (x##y)

#define __ohm_concat_string(x, y) __ohm_concat_string__(x,y)

/**
 * generate an serial name by line
 */
#define ohm_auto_name(x) __ohm_concat_string(x, __LINE__)

#endif //OMEGA_MACRO_H
