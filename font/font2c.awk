# To be executed with awk.
# Generates a data file out of a BDF font file.
BEGIN {
    count = 0;
    print("// Generated code. Do not edit.");
    print("#include \"font-data.h\"");
    print;
    print("struct Font5x8 kFontData[] = {");
}

/^STARTCHAR/              { val=$2 }
/^ENCODING/               { enc=$2; printf("  { %5d, {", enc); }
/^BITMAP/                 { in_bitmap=1; byte_count=0; }

/^[0-9a-fA-F][0-9a-fA-F]/ {
    if (in_bitmap) {
	if (byte_count > 0) { printf (","); }
	printf("0x%s", $1);
	byte_count++;
    }
}

/^ENDCHAR/                {
    printf("}}, // U+%04X %s\n", enc, val);
    in_bitmap=0; count++; 
}

END {
    printf("};\nuint32_t kFontDataSize = %i;\n", count);
}
