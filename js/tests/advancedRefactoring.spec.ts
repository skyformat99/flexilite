/**
 * Created by slanska on 2016-03-06.
 */

/// <reference path="../../typings/mocha/mocha.d.ts"/>
/// <reference path="../../typings/node/node.d.ts"/>
/// <reference path="../../typings/chai/chai.d.ts" />
/// <reference path="../../typings/tsd.d.ts" />

'use strict';

import mocha = require('mocha');

require('../lib/drivers/SQLite');
import path = require('path');
import _ = require('lodash');

describe('Advanced cases of data refactoring', () => {
    it('1. Merge objects', (done: Function) => {
        done();
    });

    it('2. Split objects', (done: Function) => {
        done();
    });

    it('3. Change class type (assign objects to a different class)', (done: Function) => {
        done();
    });

    it('4. Rename class', (done: Function) => {
        done();
    });

    it('5. Rename property', (done: Function) => {
        done();
    });

    it('6. One-to-many -> many-to-many', (done: Function) => {
        done();
    });

    /*
    Country text column -> Extract to separate object, replace with country ID -> include into row
    by auto-generated link to Countries
     */
    it('7. Scalar value(s) -> Extract to separate object -> Display value(s) from referenced object', (done: Function) => {
        done();
    });
});