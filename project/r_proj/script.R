
#File            Entropy SymbolCount
#bible.txt       4.3428  4047392
#dblp.xml.100MB  5.2279  104857600
#english.100MB   4.5559  104857600
#sources.100MB   5.5399  104857600

original.sizes <- data.frame(File=c("bible.txt","dblp.xml.100MB", "english.100MB", "sources.100MB"),
                             Size=c(4047392,104857600,104857600,104857600));
df <- read.csv2("..//benchmark_result.csv")
df <- df[df$Level==9 | df$Method == "ogrp" | df$Method == "rpse",];
df <- df[order(df$File),]
df$OS <- c(4047392,4047392,4047392,4047392,4047392,
           104857600,104857600,104857600,104857600,104857600,
           104857600,104857600,104857600,104857600,104857600,
           104857600,104857600,104857600,104857600,104857600);


df$CS <- df$OS * as.numeric(as.character(df$CR))
df$BPS <- (df$CS*8) / df$OS
df
