/*********************IMPORTANT DRAKVUF LICENSE TERMS***********************
 *                                                                         *
 * DRAKVUF (C) 2014-2016 Tamas K Lengyel.                                  *
 * Tamas K Lengyel is hereinafter referred to as the author.               *
 * This program is free software; you may redistribute and/or modify it    *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; Version 2 ("GPL"), BUT ONLY WITH ALL OF THE   *
 * CLARIFICATIONS AND EXCEPTIONS DESCRIBED HEREIN.  This guarantees your   *
 * right to use, modify, and redistribute this software under certain      *
 * conditions.  If you wish to embed DRAKVUF technology into proprietary   *
 * software, alternative licenses can be aquired from the author.          *
 *                                                                         *
 * Note that the GPL places important restrictions on "derivative works",  *
 * yet it does not provide a detailed definition of that term.  To avoid   *
 * misunderstandings, we interpret that term as broadly as copyright law   *
 * allows.  For example, we consider an application to constitute a        *
 * derivative work for the purpose of this license if it does any of the   *
 * following with any software or content covered by this license          *
 * ("Covered Software"):                                                   *
 *                                                                         *
 * o Integrates source code from Covered Software.                         *
 *                                                                         *
 * o Reads or includes copyrighted data files.                             *
 *                                                                         *
 * o Is designed specifically to execute Covered Software and parse the    *
 * results (as opposed to typical shell or execution-menu apps, which will *
 * execute anything you tell them to).                                     *
 *                                                                         *
 * o Includes Covered Software in a proprietary executable installer.  The *
 * installers produced by InstallShield are an example of this.  Including *
 * DRAKVUF with other software in compressed or archival form does not     *
 * trigger this provision, provided appropriate open source decompression  *
 * or de-archiving software is widely available for no charge.  For the    *
 * purposes of this license, an installer is considered to include Covered *
 * Software even if it actually retrieves a copy of Covered Software from  *
 * another source during runtime (such as by downloading it from the       *
 * Internet).                                                              *
 *                                                                         *
 * o Links (statically or dynamically) to a library which does any of the  *
 * above.                                                                  *
 *                                                                         *
 * o Executes a helper program, module, or script to do any of the above.  *
 *                                                                         *
 * This list is not exclusive, but is meant to clarify our interpretation  *
 * of derived works with some common examples.  Other people may interpret *
 * the plain GPL differently, so we consider this a special exception to   *
 * the GPL that we apply to Covered Software.  Works which meet any of     *
 * these conditions must conform to all of the terms of this license,      *
 * particularly including the GPL Section 3 requirements of providing      *
 * source code and allowing free redistribution of the work as a whole.    *
 *                                                                         *
 * Any redistribution of Covered Software, including any derived works,    *
 * must obey and carry forward all of the terms of this license, including *
 * obeying all GPL rules and restrictions.  For example, source code of    *
 * the whole work must be provided and free redistribution must be         *
 * allowed.  All GPL references to "this License", are to be treated as    *
 * including the terms and conditions of this license text as well.        *
 *                                                                         *
 * Because this license imposes special exceptions to the GPL, Covered     *
 * Work may not be combined (even as part of a larger work) with plain GPL *
 * software.  The terms, conditions, and exceptions of this license must   *
 * be included as well.  This license is incompatible with some other open *
 * source licenses as well.  In some cases we can relicense portions of    *
 * DRAKVUF or grant special permissions to use it in other open source     *
 * software.  Please contact tamas.k.lengyel@gmail.com with any such       *
 * requests.  Similarly, we don't incorporate incompatible open source     *
 * software into Covered Software without special permission from the      *
 * copyright holders.                                                      *
 *                                                                         *
 * If you have any questions about the licensing restrictions on using     *
 * DRAKVUF in other works, are happy to help.  As mentioned above,         *
 * alternative license can be requested from the author to integrate       *
 * DRAKVUF into proprietary applications and appliances.  Please email     *
 * tamas.k.lengyel@gmail.com for further information.                      *
 *                                                                         *
 * If you have received a written license agreement or contract for        *
 * Covered Software stating terms other than these, you may choose to use  *
 * and redistribute Covered Software under those terms instead of these.   *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes.          *
 *                                                                         *
 * Source code also allows you to port DRAKVUF to new platforms, fix bugs, *
 * and add new features.  You are highly encouraged to submit your changes *
 * on https://github.com/tklengyel/drakvuf, or by other methods.           *
 * By sending these changes, it is understood (unless you specify          *
 * otherwise) that you are offering unlimited, non-exclusive right to      *
 * reuse, modify, and relicense the code.  DRAKVUF will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).                                        *
 * To specify special license conditions of your contributions, just say   *
 * so when you send them.                                                  *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the DRAKVUF   *
 * license file for more details (it's in a COPYING file included with     *
 * DRAKVUF, and also available from                                        *
 * https://github.com/tklengyel/drakvuf/COPYING)                           *
 *                                                                         *
 ***************************************************************************/

#include <libvmi/libvmi.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <glib.h>

#include "private.h"
#include "win-offsets.h"

/* this should work for both 32 and 64bit */
#define EX_FAST_REF_MASK    7
#define HANDLE_MULTIPLIER   4

// TODO: This is a very slow PoC.. could be optimized
addr_t handle_table_get_entry(uint32_t bit, vmi_instance_t vmi,
        addr_t table_base, uint32_t level, uint32_t depth,
        uint32_t *handle_count, uint64_t handle) {

    uint32_t count;
    uint32_t table_entry_size = 0;

    if (level > 0) {
        if (bit == BIT32)
            table_entry_size = 0x4;
        else
            table_entry_size = 0x8;
    } else if (level == 0) {
        if (bit == BIT32)
            table_entry_size = 0x8;
        else
            table_entry_size = 0x10;
    };

    count = VMI_PS_4KB / table_entry_size;

    PRINT_DEBUG("\tHandle table array size: %u at 0x%lx. "
                "Table entry size is %u. Handle count remaining: %u\n",
                count, table_base, table_entry_size, *handle_count);

    uint32_t i;
    for (i = 0; i < count; i++) {

        // Only read the already known number of entries
        if (*handle_count == 0)
            break;

        addr_t table_entry_addr;
        if ( VMI_FAILURE == vmi_read_addr_va(vmi, table_base + i * table_entry_size, 0, &table_entry_addr) )
            continue;

        // skip entries that point nowhere
        if (!table_entry_addr)
            continue;

        // real entries are further down the chain
        if (level > 0) {
            addr_t next_level;
            if ( VMI_FAILURE == vmi_read_addr_va(vmi, table_base + i * table_entry_size, 0, &next_level) )
                continue;

            addr_t test = handle_table_get_entry(bit, vmi, next_level, level - 1,
                                                 depth, handle_count, handle);
            if (test)
                return test;

            depth++;
            continue;
        }

        // At this point each (table_base + i*entry) is a _HANDLE_TABLE_ENTRY

        uint32_t level_base = depth * count * HANDLE_MULTIPLIER;
        uint32_t handle_value = (i * table_entry_size * HANDLE_MULTIPLIER)
                / table_entry_size + level_base;

        PRINT_DEBUG("\t\tHandle #: %u. Addr: 0x%lx. Value: 0x%x\n",
                    *handle_count, table_entry_addr & ~EX_FAST_REF_MASK, handle_value);

        if (handle_value == handle)
            return table_entry_addr & ~EX_FAST_REF_MASK;

        // decrement the handle counter because we found one here
        --(*handle_count);
    }
    return 0;
}

addr_t drakvuf_get_obj_by_handle(drakvuf_t drakvuf, addr_t process, uint64_t handle) {

    vmi_instance_t vmi = drakvuf->vmi;
    addr_t handletable, tablecode;
    uint32_t handlecount;

    if ( VMI_FAILURE == vmi_read_addr_va(vmi, process + drakvuf->offsets[EPROCESS_OBJECTTABLE], 0, &handletable) )
        return 0;

    if ( VMI_FAILURE == vmi_read_addr_va(vmi, handletable, 0, &tablecode) )
        return 0;

    if ( VMI_FAILURE == vmi_read_32_va(vmi, handletable + drakvuf->offsets[HANDLE_TABLE_HANDLECOUNT], 0, &handlecount) )
        return 0;

    // _EX_FAST_REF-style pointer, last three bits are used for storing the number of levels
    addr_t table_base = tablecode & ~EX_FAST_REF_MASK;
    uint32_t table_levels = tablecode & EX_FAST_REF_MASK;
    uint32_t table_depth = 0;

    PRINT_DEBUG("Handle table @ 0x%lx. Handle count %u. Looking for handle: 0x%lx\n",
                table_base, handlecount, handle);

    return handle_table_get_entry(PM2BIT(drakvuf->pm), vmi, table_base,
                                  table_levels, table_depth, &handlecount, handle);
}


/////////////////////////////////////////////////////////////////////////////////////////////


bool drakvuf_obj_ref_by_handle( drakvuf_t drakvuf, drakvuf_trap_info_t *info, addr_t current_eprocess,
                                addr_t handle, object_manager_object_t obj_type_arg, addr_t *obj_body_addr )
{
    bool ret        = false ;
    addr_t obj_addr = 0 ;

    obj_addr = drakvuf_get_obj_by_handle( drakvuf, current_eprocess, handle );

    if ( obj_addr )
    {
        uint8_t object_type ;
        access_context_t ctx = {
            .translate_mechanism = VMI_TM_PROCESS_DTB,
            .dtb = info->regs->cr3,
        };

        // Get TypeIndex from _OBJ_HEADER...
        ctx.addr = obj_addr + drakvuf->offsets[ OBJECT_HEADER_TYPEINDEX ] ;

        if ( vmi_read_8( drakvuf->vmi, &ctx, &object_type ) == VMI_SUCCESS )
        {
            if ( object_type == obj_type_arg )
            {
                if ( object_type == OBJ_MANAGER_PROCESS_OBJECT )
                {
                    // Object Body must be an _EPROCESS...
                    ret = drakvuf_is_eprocess( drakvuf, info->regs->cr3, obj_addr + drakvuf->offsets[ OBJECT_HEADER_BODY ] );
                }
                else
                if ( object_type == OBJ_MANAGER_THREAD_OBJECT )
                {
                    // Object Body must be an _ETHREAD...
                    ret = drakvuf_is_ethread( drakvuf, info->regs->cr3, obj_addr + drakvuf->offsets[ OBJECT_HEADER_BODY ] );
                }
                else // Other object types...
                    ret = true ;
            }
        }
    }

    if ( ret )
    {
        *obj_body_addr = obj_addr + drakvuf->offsets[ OBJECT_HEADER_BODY ];
    }

    return ret ;
}
