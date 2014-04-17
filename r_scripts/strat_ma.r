setwd("c:/workspace")
library(zoo)
library(TTR)

file_pos <- 'o-h_ma_25-270_0.7.order'

pos <- read.csv(file_pos, header = F)
pos <- subset(pos[,c(1,2,4,6,8,9)])


funcGetRtnAnalysis = function(ret, o, h){
  
  return(c(
    o = o,
    h = h,
    ttl_rtn = sum(ret), 
    mean_rtn = mean(ret),
    sd_rtn = sd(ret),
    sharpe = mean(ret)/sd(ret), 
    critical_v = mean(ret)/sd(ret)*sqrt(length(ret)), 
    no_pos = NROW(pos),
    win_loss_ratio = NROW(ret[ret>0])/NROW(ret[ret<0])))
}

ret_temp = subset(pos, V1==25 & V2==7)
ret = (ret_temp$V8 - ret_temp$V6)*ret_temp$V9
result = funcGetRtnAnalysis(ret,25,7)


for (o in seq(5, 10, by = 1)) {  
  for(h in seq(30, 60, by = 5)){
    ret_temp = subset(pos, V1==o & V2==h)
    ret = (ret_temp$V8 - ret_temp$V6)*ret_temp$V9
    result <- rbind(result, funcGetRtnAnalysis(ret, o, h))
  }
}


write.csv(result, "result.csv")

