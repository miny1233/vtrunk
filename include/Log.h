//
// Created by 谢子南 on 2023/9/23.
//

#ifndef VTRUNK_LOG_H
#define VTRUNK_LOG_H

#define LOG(...) \
std::cout<<std::format("[{}] {}\n",__FILE_NAME__,std::format(__VA_ARGS__))

#endif //VTRUNK_LOG_H
