BEGIN {
    debug=4;
    BLANK="          ";
    TAG_RECORDING=".rec";


    # set default values if not defined
    if (basedir == "")		{basedir="/var/vdr/data/_video/";}
    if (Seperator == "")	{Seperator="|";}
    if (LenTitle == "")		{LenTitle=50;}
    if (LenGenre == "")		{LenGenre=20;}
    if (LenCountry == "")	{LenCountry=5;}
    if (LenYear == "")		{LenYear=4;}
    if (LenLength == "")	{LenLength=4;}
    if (LenDate == "")		{LenDate=10;}
    if (LenSize == "")		{LenSize=9;}
    if (LenMedia == "")		{LenMedia=8;}
}

{
    InputParam=$0
    InfoFile=InputParam;
    if (debug>0) printf("InfoFile :%s\n", InfoFile) > "/dev/stderr";
    InfoFileBasename="";

    Title="";
    SubTitle="";
    Genre="";
    Country="";
    Year="";
    Length="";
    Date="";
    Size="";
    Media="Internal";
    Path="";

    Len="";
    Nr="----";
    FullDir="";

    n = split (InfoFile, p, "/");
    if (debug>2) {
	for (i=1; i<=n; i++) {
	    printf("p[%d]     :%s\n", i, p[i]) > "/dev/stderr";
	}
    }
    if (n>2) {
	Dir = p[1];
	FullDir = p[1];
	if (debug>2) printf("FullDir1 :%s\n", FullDir) > "/dev/stderr";
	InfoFileBasename = p[n];
	for (i=2; i<=n-1; i++) {
	    if (i<=n-2) {
		Dir = Dir "/" p[i];
		if (debug>2) printf("Dir    2 :%s\n", Dir) > "/dev/stderr";
	    } else {
		Date = substr(p[i],1, 10);
		if (debug>2) printf("Date     :%s\n", Date) > "/dev/stderr";
	    }
	    FullDir = FullDir "/" p[i];
	    if (debug>2) printf("FullDir2 :%s\n", FullDir) > "/dev/stderr";
	}
	if (index(Dir, basedir)==1) {
	    Path = substr(Dir, length(basedir)+1);
	    if (debug>2) printf("Path     :%s\n", Path) > "/dev/stderr";
	}
    }
    if (FullDir != "") {
	EscDir = FullDir;
	gsub(/\"/, "\\\"", EscDir);
	if (debug>0) printf("EscDir   :%s\n", EscDir) > "/dev/stderr";
	getsize = "du \""EscDir"\"";
	if ((getsize | getline duRes) > 0) {
	    n = split(duRes, s, " ");
	    Size = s[1];
	    if (debug>1) printf("Size     :%s\n", Size) > "/dev/stderr";
	    close(getsize)
	}
    }


    # check, if recording is valid (not deleted)
    if (match(EscDir, /rec$/)) {

	# check for length file
	LenFile=InputParam;
	sub( InfoFileBasename, "length.vdr", LenFile );
	if (debug>0) printf("Looking for length...\n") > "/dev/stderr";
	if (debug>1) printf("LenFile1 :%s\n", LenFile) > "/dev/stderr";
	getline Len < LenFile;
	if (debug>1) printf("Len    1 :%s\n", Len) > "/dev/stderr";
	if (Len != "") {
	    Length = sprintf("%d:%02d", Len/60, Len%60);
	} else {
	    IndexFile=InputParam;
	    sub( InfoFileBasename, "index.vdr", IndexFile );
	    gsub(/\"/, "\\\"", IndexFile);
	    if (debug>1) printf("IndexFile:%s\n", IndexFile) > "/dev/stderr";
	    $1="";
	    "stat -L -c %s \""IndexFile"\" 2>/dev/null" | getline;
	    if (debug>2) printf("Len    2 :%s\t%s\n", $1, IndexFile) > "/dev/stderr"; 
	    if ($1 != "") {
		Len=($1/12000); 
		if (debug>1) printf("Len    3 :%s\n", Len) > "/dev/stderr"; 
		Length = sprintf("%d:%02d", Len/60, Len%60);
	    }
	}


	# check for archive DVD
	ExtFile=InputParam;
	sub( InfoFileBasename, "dvd.vdr", ExtFile );
	if (debug>0) printf("Looking for archive DVD...\n") > "/dev/stderr";
	if (debug>1) printf("ExtFile1 :%s\n", ExtFile) > "/dev/stderr";
	getline Nr < ExtFile;
	if (debug>1) printf("Nr     1 :%s\n", Nr) > "/dev/stderr";
	if (Nr != "----") {
	    Media="DVD "Nr;
	} else {
	    # check for archive HDD
	    ExtFile=InputParam;
	    sub( InfoFileBasename, "hdd.vdr", ExtFile );
	    if (debug>0) printf("Looking for archive HDD...\n") > "/dev/stderr";
	    if (debug>1) printf("ExtFile2 :%s\n", ExtFile) > "/dev/stderr";
	    getline Nr < ExtFile;
	    if (debug>1) printf("Nr     2 :%s\n", Nr) > "/dev/stderr";
	    if (Nr != "----") {
		Media="HDD "Nr;
	    }
	}


	do {
	    rc=getline line < InfoFile;
	    if ( rc>0 ) {
		if ( substr(line, 1, 1)=="T" ) {
		    Title=substr(line, 3);
		} else if ( substr(line, 1, 1)=="S" ) {
		    SubTitle=substr(line, 3);
		} else if ( substr(line, 1, 1)=="D" ) {

		    # D Horrorfilm, USA  1999||...
		    line=substr(line, 3);
		    # Horrorfilm, USA  1999||...
		    if (debug>2) printf("line3:%s\n", line) > "/dev/stderr";

		    if (debug>0) printf("Looking for values...\n") > "/dev/stderr";

		    # get Genre from tag (...|Genre: xxx|...)
		    Genre2=line;
		    if (debug>0) printf("Looking for Genre... ( tag '...|Genre: xxx|...' )\n") > "/dev/stderr";
		    startIndex=index(Genre2, "|Genre: ");
		    if (startIndex>0) {
			Genre3=substr(Genre2, startIndex+8);
			if (debug>2) printf("c3 :%s\n", Genre3) > "/dev/stderr";
			stopIndex=index(Genre3, "|");
			if (stopIndex>1) {
			    Genre4=substr(Genre3, 0, stopIndex-1);
			    Genre=Genre4;
			    if (debug>1) printf("Found Genre: %s\n", Genre) > "/dev/stderr";
			}
		    }

		    # get Year from tag (...|Year: xxx|...)
		    Year2=line;
		    if (debug>0) printf("Looking for Year... ( tag '...|Year: xxx|...' )\n") > "/dev/stderr";
		    startIndex=index(Year2, "|Year: ");
		    if (startIndex>0) {
			Year3=substr(Year2, startIndex+7);
			if (debug>2) printf("y3 :%s\n", Year3) > "/dev/stderr";
			stopIndex=index(Year3, "|");
			if (stopIndex>1) {
			    Year4=substr(Year3, 0, stopIndex-1);
			    Year=Year4;
			    if (debug>1) printf("Found Year: %s\n", Year) > "/dev/stderr";
			}
		    }

		    # get Country (and Year if not already filled) from tag (...|Land: XXX  YYYY|...)
		    if(Country=="" || Year=="") {
			Country2=line;
			if (debug>0) printf("Looking for Country or Year... ( tag '...|Land: XXX  YYYY|...' )\n") > "/dev/stderr";
			startIndex=index(Country2, "|Land: ");
			if (startIndex>0) {
			    Country3=substr(Country2, startIndex+7);	# USA  1999|...
			    if (debug>2) printf("c3 :%s\n", Country3) > "/dev/stderr";
			    stopIndex=index(Country3, "|");
			    if (stopIndex>1) {
				Country4=substr(Country3, 0, stopIndex-1);	# USA  1999
				if (debug>2) printf("c4 :%s\n", Country4) > "/dev/stderr";
				gsub(/^ +/, "", Country4);
				gsub(/ +$/, "", Country4);
				gsub(/ +/, " ", Country4);	# USA 1999
				if (debug>2) printf("c5 :%s\n", Country4) > "/dev/stderr";
				n = split (Country4, F2, / *[, ] */ ); # split to 'USA' '1999'
				for (i=1; i<=n; i++) {
				    if (match (F2[i], /^[0-9]+$/)) {	# only digits
					if (Year=="") {
					    Year = F2[i];
					    if (debug>1) printf("Found Year: %s\n", Year) > "/dev/stderr";
					}
				    } else {
					if (match (F2[i], /^[A-Z/]+$/)) {	# only UPPERCASE
					    if (Country=="") {
						Country = F2[i];
						if (debug>1) printf("Found Country: %s\n", Country) > "/dev/stderr";
					    }
					}
				    }
				}
			    }
			}
		    }


		    # if not filled already, get Genre, Country and Year from the beginning of the line (Horrorfilm, USA  1999||...)
		    if(Genre=="" || Country=="" || Year=="") {
			if (debug>0) printf("Looking for Genre, Country or Year... ( tag 'D Horrorfilm, USA  1999||...' )\n") > "/dev/stderr";
			n = split (line,  F1, "|");
			# F1[1]: Horrorfilm, USA  1999
			gsub(/^ +/, "", F1[1]);
			gsub(/ +$/, "", F1[1]);
			if (debug>2) printf("b1: F1[1]:%s\n", F1[1]) > "/dev/stderr";
			n = split (F1[1], F2, / *[, ] */ ); # split to 'Horrorfilm' 'USA' '1999'
			for (i=1; i<=n; i++) {
			    if (debug>2) printf("b2: F2[%d]:%s\n", i, F2[i]) > "/dev/stderr";
			    if (match (F2[i], /^[0-9]+$/)) {	# only digits
				if (Year=="") {
				    Year = F2[i];
				    if (debug>1) printf("Found Year: %s\n", Year) > "/dev/stderr";
				}
			    } else {
				if (match (F2[i], /^[A-Z/]+$/)) {	# only UPPERCASE
				    if (Country=="") {
					Country = F2[i];
					if (debug>1) printf("Found Country: %s\n", Country) > "/dev/stderr";
				    }
				} else {
				    if (Genre=="") {
					Genre = F2[i];
					if (debug>1) printf("Found Genre: %s\n", Genre) > "/dev/stderr";
				    }
				}
			    }
			}
		    }

		} else {
		    if (debug>2) printf("l :%s\n", line) > "/dev/stderr";
		}
	    }
	} while (rc>0);
#        close("/dev/stderr");

	if (debug>0) {
	    printf("Title    :%s\n", Title) > "/dev/stderr";
	    printf("SubTitle :%s\n", SubTitle) > "/dev/stderr";
	    printf("Genre    :%s\n", Genre) > "/dev/stderr";
	    printf("Country  :%s\n", Country) > "/dev/stderr";
	    printf("Year     :%s\n", Year) > "/dev/stderr";
	    printf("Length   :%s\n", Length) > "/dev/stderr";
	    printf("Date     :%s\n", Date) > "/dev/stderr";
	    printf("Size     :%s\n", Size) > "/dev/stderr";
	    printf("Media    :%s\n", Media) > "/dev/stderr";
	    printf("Path     :%s\n\n\n", Path) > "/dev/stderr";
	}

	if (SubTitle != "") {
	    Title = Title": "SubTitle;
	}
	if (length(Title)>LenTitle) {	Title = substr(Title, 1, LenTitle) };
	printf("%-"LenTitle"s%s", Title, Seperator);

	if (length(Genre)>LenGenre) {	Genre = substr(Genre, 1, LenGenre) };
	printf("%-"LenGenre"s%s", Genre, Seperator);

	if (length(Media)>LenCountry) {	Country = substr(Country, 1, LenCountry) };
	printf("%-"LenCountry"s%s", Country, Seperator);

	if (length(Year)>LenYear) {	Year = substr(Year, length(Year)-LenYear+1, LenYear) };
	printf("%"LenYear"s%s", Year, Seperator);

	if (length(Media)>LenLength) {	Length = substr(Length, length(Length)-LenLength+1, LenLength) };
	printf("%-"LenLength"s%s", Length, Seperator);

	if (length(Date)>LenDate) {	Date = substr(Date, length(Date)-LenDate+1, LenDate) };
	printf("%"LenDate"s%s", Date, Seperator);

	if (length(Size)>LenSize) {	Size = substr(Size, length(Size)-LenSize+1, LenSize) };
	printf("%"LenSize"s%s", Size, Seperator);

	if (length(Media)>LenMedia) {	Media = substr(Media, length(Media)-LenMedia+1, LenMedia) };
	printf("%-"LenMedia"s%s", Media, Seperator);

	printf("%s\n", Path);
    } else {
	if (debug>1) printf("No regular recording. Ignoring.\n") > "/dev/stderr";
    }
}

END {
}