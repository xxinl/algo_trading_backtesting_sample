setwd("c:/workspace")
library(zoo)
library(TTR)

file_pos <- '20140416.085052.order'

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

ret_temp = subset(pos, V1==10 & V2==60)
ret = (ret_temp$V8 - ret_temp$V6)*ret_temp$V9
result = funcGetRtnAnalysis(ret,10,60)

# for (o in 1:10) {  
#   for(h in o + 1:(120- o - 1)){
#     ret_temp = subset(pos, V1==o & V2==h)
#     ret = (ret_temp$V8 - ret_temp$V6)*ret_temp$V9
#     result <- rbind(result, funcGetRtnAnalysis(ret, o, h))
#   }
# }

for (o in seq(5, 125, by = 15)) {  
  for(h in seq(20, 1000, by = 20)){
    ret_temp = subset(pos, V1==o & V2==h)
    ret = (ret_temp$V8 - ret_temp$V6)*ret_temp$V9
    result <- rbind(result, funcGetRtnAnalysis(ret, o, h))
  }
}


write.csv(result, "result.csv")

