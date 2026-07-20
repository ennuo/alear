const { execSync } = require('child_process');
const { readFileSync, rmSync, writeFileSync, existsSync, unlinkSync } = require('fs');
const { deflateSync, constants } = require('zlib');
const { compress } = require('e:/cod/library/src/memory/compressor');

const HEADER_SIZE = 0x30;
const NOP = 0x4020007f;

const SPU_I18 = 0x01ffff80;
const SPU_I16 = 0x007fff80;

const SHT_SYMTAB = 0x2;
const SHT_STRTAB = 0x3;

execSync('spu-lv2-gcc -c -fpic -fno-jump-tables -O3 -nostdlib ../../code/CWLib/PartPhysicsJointShared.cpp -I../../code/CoreLib -I../../code/CWLib -DSPU');
const binary = readFileSync('PartPhysicsJointShared.o');
rmSync('PartPhysicsJointShared.o');

const sectionHeaderData = binary.readUint32BE(0x20);
const numSections = binary.readUint16BE(0x30);

const sections = [];
for (let i = 0; i < numSections; ++i)
{
    const offset = sectionHeaderData + (i * 0x28);
    const section = 
    {
        name: binary.readUint32BE(offset + 0x00),
        type: binary.readUint32BE(offset + 0x04),
        offset: binary.readUint32BE(offset + 0x10),
        size: binary.readUint32BE(offset + 0x14)
    };

    sections.push(section);
}

const sectionHeaderStringTable = sections[binary.readUint16BE(0x32)];
for (const section of sections)
{
    section.name = binary.subarray(
        sectionHeaderStringTable.offset + section.name,
        binary.indexOf('\0', sectionHeaderStringTable.offset + section.name)
    ).toString('ascii');
}

const textSection = sections.find(x => x.name == '.text');
const symbolSection = sections.find(x => x.type == SHT_SYMTAB);
const stringTableSection = sections.find(x => x.name == '.strtab');

const symbols = [];

for (let i = 0; i < symbolSection.size / 16; ++i)
{
    const offset = symbolSection.offset + (i * 16);

    const shn = binary.readUint16BE(offset + 14);
    let name = '';
    if (shn != 0)
    {
        const nameData = stringTableSection.offset + binary.readUint32BE(offset);
        name = binary.subarray(nameData, binary.indexOf('\0', nameData)).toString('ascii');
    }

    const symbol =
    {
        name,
        value: binary.readUint32BE(offset + 4),
        size: binary.readUint32BE(offset + 8),
        info: binary[offset + 12],
        other: binary[offset + 13],
        shndx: binary.readUint16BE(offset + 14)
    };

    symbols.push(symbol);
}

function getSymbolData(name)
{
    const sym = symbols.find(x => x.name == name);
    const start = sections[sym.shndx].offset + sym.value;
    return binary.subarray(start, start + sym.size);
}

const code_GetDesiredLengthVel = getSymbolData('_ZNK6PJoint19GetDesiredLengthVelEv');
const code_GetDesiredLength = getSymbolData('_ZNK6PJoint16GetDesiredLengthEf');

const addr_GetDesiredLengthVel = 0x270;
const addr_GetDesiredLength = 0x3040;


let spu = readFileSync('readjoint.mod');
let bssAddress = spu.readUint32BE(0x14);
if (spu.length < bssAddress) spu = Buffer.concat([ spu, Buffer.alloc(bssAddress - spu.length) ]);
const hookDataOffset = spu.length;

const relaSection = sections.find(x => x.name == '.rela.text');
if (relaSection)
{
    console.log('RELOCATIONS!');

    const numRelocations = relaSection.size / 0xc;
    for (let i = 0; i < relaSection.size / 0xc; ++i)
    {
        const offset = relaSection.offset + (i * 0xc);
        const rela =
        {
            offset: binary.readUInt32BE(offset),
            type: binary.readUInt32BE(offset + 4) & 0xff,
            symbolIndex: binary.readUint32BE(offset + 4) >> 8,
            addend: binary.readInt32BE(offset + 8)
        };

        const symbol = symbols[rela.symbolIndex];
        const symbolValue = symbol.value;

        const oldValue = binary.readInt32BE(textSection.offset + rela.offset);



        let newValue = 0;
        switch (rela.type)
        {
            case 5: // SPU_ADDR18
            {
                console.log(`SPU_ADDR18 : ${hookDataOffset + rela.offset}`);
                newValue = symbolValue + hookDataOffset + rela.addend;
                newValue = (oldValue & ~SPU_I18) | (newValue << 7);
                break;
            }
            case 7: // SPU_REL16
            {
                newValue = (symbolValue + rela.addend - rela.offset) >> 2;
                newValue = (oldValue & ~SPU_I16) | ((newValue << 7) & SPU_I16);
                break;
            }
            case 17:
            {
                newValue = oldValue;
                // newValue = (oldValue & ~0xffff0000) | (0x1c << 24);
                // newValue = (newValue & ~0x0000ff00) | (newValue & 0x00003f00);
                break;
            }
            default:
            {
                console.log('unsupported relocation type : ' + rela.type);
                throw new Error('FUCK!');
            }
        }

        binary.writeInt32BE(newValue, textSection.offset + rela.offset);
    }
}

spu = Buffer.concat([ spu, binary.subarray(textSection.offset, textSection.offset + textSection.size )]);
if ((spu.length % 16) != 0) spu = Buffer.concat([ spu, Buffer.alloc(16 - (spu.length % 16 ))]);

// This shouldn't really matter because the entrypoint will
// still be before our own data, and the bss data is usually never
// used.
bssAddress = spu.length;
if (bssAddress % 16) bssAddress += (16 - (bssAddress % 16));

const bssSize = spu.readUint32BE(0x18);
const entryPoint = spu.readUint32BE(0x10);

spu.writeUint32BE(bssAddress, 0x14);
spu.writeUint32BE((spu.readUint32BE(entryPoint + (15 * 4)) & ~SPU_I18) | (bssAddress << 7), entryPoint + (15 * 4));
spu.writeUint32BE((spu.readUint32BE(entryPoint + (16 * 4)) & ~SPU_I18) | ((bssAddress + bssSize) << 7), entryPoint + (16 * 4));

function hook(addr, symbolName)
{
    const symbol = symbols.find(x => x.name == symbolName);
    const target = hookDataOffset + symbol.value;

    for (let i = 0x30; i < hookDataOffset; i += 0x4)
    {
        const instruction = spu.readUInt32BE(i);
        if ((instruction & ~SPU_I18) == 0x32000000)
        {
            let value = ((instruction >>> 5) & 0x3fffc);
            if (value & (1 << 17))
                value = -((~value & 0x3ffff) + 1);
            value += i;


            if (value == addr)
            {
                console.log(`found call to ${symbolName} @ 0x${i.toString(16)}`);
                const delta = target - i;
                // always positive so whatever
                spu.writeUint32BE(
                    (instruction & ~0x7fffe0) | delta << 5 | (instruction & 0x1f),
                    i
                );
            }
        }
    }

    // spu.writeUint32BE(0x42000002 | (target << 7), addr);
    // spu.writeUint32BE(0x35000100, addr + 4);
}

hook(addr_GetDesiredLengthVel, '_ZNK6PJoint19GetDesiredLengthVelEv');
// hook(addr_GetDesiredLength, '_Z16GetDesiredLengthPK6PJointf');
hook(addr_GetDesiredLength, '_ZNK6PJoint16GetDesiredLengthEf');



function split(buffer, size) 
{
    let count = Math.ceil(buffer.length / size);
    let chunks = [];
    for (let i = 0, offset = 0; i < count; i++, offset += size) 
    {
        let end = Math.min(buffer.length, offset + size);
        chunks.push(buffer.slice(offset, end));
    }
    return chunks;
}

const chunks = split(spu, 0x8000);
const compressedSize = [];
const uncompressedSize = [];

let zlibDataSize = 0;
const zlibStreams = [];

for (const chunk of chunks)
{
    const compressed = deflateSync(chunk, {
        level: constants.Z_BEST_COMPRESSION
    });

    zlibStreams.push(compressed);
    compressedSize.push(compressed.length);
    uncompressedSize.push(chunk.length);
    zlibDataSize += compressed.length;
}

const output = Buffer.alloc(4 + (chunks.length) * 0x4);
output.writeUint16BE(1, 0x0);
output.writeUint16BE(zlibStreams.length, 0x2);
for (let i = 0; i < zlibStreams.length; ++i)
{
    output.writeUint16BE(compressedSize[i], 0x4 + (i * 0x4) + 0x00);
    output.writeUint16BE(uncompressedSize[i], 0x4 + (i * 0x4) + 0x02);
}

writeFileSync('readjoint.sbu.mod', spu);
spu = Buffer.concat([output, ...zlibStreams]);
writeFileSync('readjoint.sbu', spu);
writeFileSync("T:\\ardis\\rpcs3\\dev_hdd0\\game\\LBP1DEBUG\\USRDIR\\gamedata\\spu\\readjoint.sbu", spu);