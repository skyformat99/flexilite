---
--- Created by slanska.
--- DateTime: 2017-12-25 10:37 AM
---

--[[Test bit 52 operations]]
require 'util'
local Util64 = require 'Util'

--[[
Python code to generate:
#python 3.5.2

import random

for x in range(10):
    a = random.randint(1 << 31, 1 << 52)
    b = random.randint(1 << 31, 1 << 52)
    i = random.randint(1, 1 << 12)
    s = random.randint(13, 40)
    mask1 = 0xFFFF00FFFFFFF
    mask2 = 0xFFFFF000FFFFF
    print("{", a, b, a & b, a | b, ~a, i, s, i << s, mask1, (a & mask1) | b, mask2, (a & mask2) | b, "}")

sample result:

a= 4432966598464300 b= 2434236601668619 a & b= 2434045331968008 a | b= 4433157868164911 ~a= -4432966598464301
i= 3198 s= 32 i << s= 13735305412608
mask1= 4503531176329215 (a & mask1) | b= 4433148741359407
mask2= 4503595333451775 (a & mask2) | b= 4433157331293999
a= 214986141353494 b= 4300080714370536 a & b= 73143495705600 a | b= 4441923360018430 ~a= -214986141353495
i= 2082 s= 40 i << s= 2289183209029632
mask1= 4503531176329215 (a & mask1) | b= 4441917722873854
mask2= 4503595333451775 (a & mask2) | b= 4441922009452542
a= 518742632227610 b= 3337853181535532 a & b= 514305155047688 a | b= 3342290658715454 ~a= -518742632227611
i= 2040 s= 19 i << s= 1069547520
mask1= 4503531176329215 (a & mask1) | b= 3342251467138878
mask2= 4503595333451775 (a & mask2) | b= 3342289882769214
a= 3993602732488703 b= 511408997086598 a & b= 17630976685446 a | b= 4487380752889855 ~a= -3993602732488704
i= 1208 s= 36 i << s= 83013127897088
mask1= 4503531176329215 (a & mask1) | b= 4487380484454399
mask2= 4503595333451775 (a & mask2) | b= 4487380482357247
a= 1350781834209838 b= 2654821422201845 a & b= 84126720174628 a | b= 3921476536237055 ~a= -1350781834209839
i= 603 s= 21 i << s= 1264582656
mask1= 4503531176329215 (a & mask1) | b= 3921458819496959
mask2= 4503595333451775 (a & mask2) | b= 3921475997268991
a= 3798107742450557 b= 2089799539027291 a & b= 1526162371944793 a | b= 4361744909533055 ~a= -3798107742450558
i= 385 s= 18 i << s= 100925440
mask1= 4503531176329215 (a & mask1) | b= 4361734172114815
mask2= 4503595333451775 (a & mask2) | b= 4361742749466495
a= 1374406405575698 b= 3025917602695219 a & b= 211115989794834 a | b= 4189208018476083 ~a= -1374406405575699
i= 3086 s= 32 i << s= 13254269075456
mask1= 4503531176329215 (a & mask1) | b= 4189201039154227
mask2= 4503595333451775 (a & mask2) | b= 4189205199903795
a= 3336217101288837 b= 3834970819397200 a & b= 2702876893905920 a | b= 4468311026780117 ~a= -3336217101288838
i= 212 s= 19 i << s= 111149056
mask1= 4503531176329215 (a & mask1) | b= 4468289551943637
mask2= 4503595333451775 (a & mask2) | b= 4468310992177109
a= 4009160857460147 b= 2649846094954079 a & b= 2298000776928275 a | b= 4361006175485951 ~a= -4009160857460148
i= 3085 s= 20 i << s= 3234856960
mask1= 4503531176329215 (a & mask1) | b= 4360961078329343
mask2= 4503595333451775 (a & mask2) | b= 4361004015419391
a= 3186202858745320 b= 938670631986243 a & b= 934125482583104 a | b= 3190748008148459 ~a= -3186202858745321
i= 2021 s= 35 i << s= 69441031241728
mask1= 4503531176329215 (a & mask1) | b= 3190745323793899
mask2= 4503595333451775 (a & mask2) | b= 3190745323793899

]]

-- [1] a, [2] b, [3] a & b, [4] a | b, [5] ~a, [6] i, [7] s, [8] i << s, [9] mask1, [10] (a & mask1) | b, [11] mask2, [12] (a & mask2) | b
local sample_data = {
    { 852489801246802, 4424854684176739, 844706309341250, 4432638176082291, -852489801246803, 3288, 29, 1765231558656, 4503531176329215, 4432620190906739, 4503595333451775, 4432637165255027 },
    { 6014191225158, 222812356991050, 137592260674, 228688955955534, -6014191225159, 1691, 37, 232409270321152, 4503531176329215, 228653522475342, 4503595333451775, 228687872776526 },
    { 3410323222762507, 2376081780081763, 2270528022840323, 3515876980003947, -3410323222762508, 1253, 22, 5255462912, 4503531176329215, 3515863021360235, 4503595333451775, 3515875906262123 },
    { 1224161903659988, 1659811011322957, 1197463812194372, 1686509102788573, -1224161903659989, 114, 35, 3917010173952, 4503531176329215, 1686474206179293, 4503595333451775, 1686508528168925 },
    { 71452184061679, 3832511957878718, 704375195310, 3903259766745087, -71452184061680, 2398, 20, 2514485248, 4503531176329215, 3903224333264895, 4503595333451775, 3903258659448831 },
    { 3413333138754114, 3312968763655508, 2251939468034112, 4474362434375510, -3413333138754115, 1435, 38, 394449796464640, 4503531176329215, 4474328074637142, 4503595333451775, 4474362280234838 },
    { 2524597890988845, 4305886740975825, 2331008676532225, 4499475955432445, -2524597890988846, 3504, 37, 481586092965888, 4503531176329215, 4499469512981501, 4503595333451775, 4499473802705917 },
    { 3719650568804050, 2213536877466720, 1430465202820160, 4502722243450610, -3719650568804051, 1051, 27, 141062832128, 4503531176329215, 4502720095966962, 4503595333451775, 4502720086529778 },
    { 1239989617568639, 101733212679243, 75327619475531, 1266395210772351, -1239989617568640, 1122, 34, 19275813224448, 4503531176329215, 1266390915805055, 4503595333451775, 1266395200286591 },
    { 3178719185030556, 2307280487966578, 2254020743873808, 3231978929123326, -3178719185030557, 828, 17, 108527616, 4503531176329215, 3231969802317822, 4503595333451775, 3231978357649406 }
}

describe('Bit52 tests', function()

    it('BOr64', function()
        for _, v in ipairs(sample_data) do
            local a = v[1]
            local b = v[2]
            local c = Util64.BOr64(a, b)
            assert.are.equal(v[4], c)
        end
    end )

    it('BAnd64', function()
        for _, v in ipairs(sample_data) do
            local a = v[1]
            local b = v[2]
            local c = Util64.BAnd64(a, b)
            assert.are.equal(v[3], c)
        end
    end )

    it('BNot64', function()
        for _, v in ipairs(sample_data) do
            local a = v[1]
            local c = Util64.BNot64(a)
            assert.are.equal(v[5], c)
        end
    end )

    it('BSet64', function()
        for _, v in ipairs(sample_data) do
            local a = v[1]
            local mask1 = v[9]
            local mask2 = v[11]
            local b = v[2]
            local c = Util64.BSet64(a, mask1, b)
            assert.are.equal(v[10], c)
            c = Util64.BSet64(a, mask2, b)
            assert.are.equal(v[12], c)
        end
    end )

    it('BLShift64', function()
        for _, v in ipairs(sample_data) do
            local i = v[6]
            local s = v[7]
            local c = Util64.BLShift64(i, s)
            assert.are.equal(v[8], c)
        end
    end)

end )