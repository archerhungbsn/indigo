/****************************************************************
 *
 *        Copyright 2014, Big Switch Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 ****************************************************************/

#include "table.h"
#include <AIM/aim_memory.h>
#include <AIM/aim_string.h>
#include "ft.h"
#include "ofstatemanager_decs.h"
#include "ofstatemanager_log.h"

static ind_core_table_t *ind_core_tables[256];
int ind_core_num_tables_registered;

ind_core_table_t *
ind_core_table_get(uint8_t table_id)
{
    return ind_core_tables[table_id];
}

void indigo_core_table_register(uint8_t table_id, const char *name,
                                const indigo_core_table_ops_t *ops, void *priv)
{
    AIM_TRUE_OR_DIE(strlen(name) <= OF_MAX_TABLE_NAME_LEN);

    list_links_t *cur, *next;
    ft_entry_t *entry;
    FT_ITER(ind_core_ft, entry, cur, next) {
        if (entry->table_id == table_id) {
            ind_core_flow_entry_delete(entry, OF_FLOW_REMOVED_REASON_DELETE,
                                       INDIGO_CXN_ID_UNSPECIFIED);
        }
    }

    ind_core_table_t *table = aim_zmalloc(sizeof(*table));
    strncpy(table->name, name, sizeof(table->name)-1);
    table->ops = ops;
    table->priv = priv;
    table->num_flows = 0;

    AIM_TRUE_OR_DIE(ind_core_tables[table_id] == NULL);
    ind_core_tables[table_id] = table;
    ind_core_num_tables_registered++;

    AIM_LOG_VERBOSE("Registered flowtable \"%s\" with table id %d", name, table_id);
}

void indigo_core_table_unregister(uint8_t table_id)
{
    ind_core_table_t *table = ind_core_tables[table_id];
    AIM_TRUE_OR_DIE(table != NULL);

    list_links_t *cur, *next;
    ft_entry_t *entry;
    FT_ITER(ind_core_ft, entry, cur, next) {
        if (entry->table_id == table_id) {
            ind_core_flow_entry_delete(entry, OF_FLOW_REMOVED_REASON_DELETE,
                                       INDIGO_CXN_ID_UNSPECIFIED);
        }
    }

    aim_free(table);
    ind_core_tables[table_id] = NULL;
    ind_core_num_tables_registered--;
}
