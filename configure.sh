#!/usr/bin/env bash

unameOut=`uname -s`
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

if [[ $machine -eq "Linux" ]]; then
	mkdir -p ./logs;
	if [[ ! -z $(command -v apt-get) ]]; then
		sudo apt-get update;
		sudo apt-get install -y gcc make libncurses5-dev libncursesw5-dev	\
								libevent-dev libreadline-dev;
	elif [[ ! -z $(command -v pkg) ]]; then
		pkg update -y;
		pkg install -y clang make ncurses-dev libevent-dev readline-dev;
	fi
elif [[ $machine -eq "Mac" ]]; then
	mkdir -p ./logs;
	brew update;
	curl -fsSL https://rawgit.com/kube/42homebrew/master/install.sh | zsh;
	source ~/.zshrc;
	brew install libevent readline ncurses;
	brew link readline libevent ncurses;
	echo "export CPATH=\"$CPATH:$HOME/.brew/opt/readline/include/:$HOME/.brew/opt/ncurses/include/:$HOME/.brew/opt/libevent/include\"" >> ~/.zshrc;
	echo "export LIBRARY_PATH=\"$HOME/.brew/opt/readline/lib:$HOME/.brew/opt/ncurses/lib/:$HOME/.brew/opt/libevent/lib\"" >> ~/.zshrc;
else
	echo "Your system doesn't supported!";
fi
