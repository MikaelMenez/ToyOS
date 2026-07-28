#ifndef USERMODE_H
#define USERMODE_H

#include "stdint.h"

/** usermode_setup:
 *  Capítulo 11.2 - Setting Up For User Mode
 *
 *  Copia o binário do módulo de usuário para uma região física dedicada
 *  (0x00400000, separada do kernel) e monta o page directory que o
 *  processo de usuário vai usar, com a entrada 0 mapeando essa região
 *  em modo usuário (U/S=1) e a entrada 768 preservando o acesso do
 *  kernel (U/S=0, igual ao page directory original).
 *
 *  @param module_start_phys endereço físico onde o GRUB carregou o módulo
 *  @param module_end_phys    endereço físico do fim do módulo
 *  @return o endereço FÍSICO do page directory do usuário (para cr3)
 */
uint32_t usermode_setup(uint32_t module_start_phys, uint32_t module_end_phys);

#endif /* USERMODE_H */