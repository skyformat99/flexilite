//
// Created by slanska on 2017-02-12.
//

#ifndef FLEXILITE_FLEXI_CLASS_H
#define FLEXILITE_FLEXI_CLASS_H

void flexi_class_create_func(
        sqlite3_context *context,
        int argc,
        sqlite3_value **argv
);

void flexi_class_alter_func(
        sqlite3_context *context,
        int argc,
        sqlite3_value **argv
);

void flexi_class_drop_func(
        sqlite3_context *context,
        int argc,
        sqlite3_value **argv
);

void flexi_class_rename_func(
        sqlite3_context *context,
        int argc,
        sqlite3_value **argv
);

#endif //FLEXILITE_FLEXI_CLASS_H
