#!/usr/bin/bash
foldername="ferry2"
num_run=1
simtime=200
mode="ON"
quickhello="QuickHello_ON"
eraseinfo="EraseBlock_ON"
user_num="500"
z=1
A_d=20000
#p_mode = 2 #1=simple 2=DTN
cd ~/ns-allinone-3.35/ns-3.35
for((i=0;i<2;i++))
do
    quickhello="QuickHello_ON"
    for ((x=0;x<2;x++))
    do
        eraseinfo="EraseBlock_ON"
        for((y=0;y<2;y++))
        do
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/ClusterViewer
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Time
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/SectionData
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Result
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/ClusterInfo/ClusterHopInfo
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/ClusterInfo/ClusterMemberSize
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/ClusterInfo/ClusterMemberAverage
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Escape
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/UserInfo
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Recive
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/testLog
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Info/Exit
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Info/Block
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Info/Speed
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/Info/Flow_moniter
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/AoI_Info
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/HelloTimeInfo/HelloRecv
            mkdir -p obayashiIOFiles/Log/obayashi/$mode/$quickhello/$eraseinfo/$user_num/HelloTimeInfo/HelloSend
            mkdir -p obayashiIOFiles/zip/obayashi/$mode/$quickhello/$eraseinfo
            eraseinfo="EraseBlock_OFF"
        done
        quickhello="QuickHello_OFF"

    done
    mode="OFF"
done

for ((i=1; i<$num_run+1 ;i++))
do

    ./waf --run "$foldername --seed=1 --run=100 --simtime=$simtime --mode=ON --usequickhello=ON --useexitinfo=ON --useeraseinfo=ON --A_d=$A_d --usehop=OFF" --gdb

    # gnome-terminal -- ./waf --run "$foldername --seed=1 --run=9 --simtime=$simtime --mode=ON --usequickhello=OFF --useexitinfo=OFF --useeraseinfo=OFF --A_d=$A_d --usehop=OFF"

    z=$((z+1))
    A_d=$((A_d+200))

done
