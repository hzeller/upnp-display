# To be executed with awk.
# Generates a data file
BEGIN { i=0; printf("// Generated code. Do not edit.\n" \
                    "#include \"font-data.h\"\n\n" \
                    "struct Font5x8 kFontData[] = {\n");
}
/^STARTCHAR/              { val=$2 }
/^ENCODING/               { printf("\t{ %5d, { ", $2); }
/^BITMAP/                 { in_bitmap=1; }
/^[0-9a-fA-F][0-9a-fA-F]/ { if (in_bitmap) printf("0x%s, ", $1); }
/^ENDCHAR/                { printf("}},  // %s\n", val); in_bitmap=0; i++; }
END { printf("};\nuint32_t kFontDataSize = %i;\n", i) }
