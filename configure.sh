#!/usr/bin/env bash

unameOut=`uname -s`
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

unameOut=`uname -o 2>/dev/null`
case "${unameOut}" in
    GNU\/Linux*) opsystem=Linux;;
    Android*)   opsystem=Android;;
    *)          opsystem="UNKNOWN:${unameOut}"
esac

if [[ $machine == "Linux" ]]; then
	mkdir -p ./logs;

	if [[ $opsystem == "Linux" ]]; then
		sudo apt-get update;
		sudo apt-get install -y gcc make libncurses5-dev libncursesw5-dev	\
								libevent-dev libreadline-dev;
	elif [[ $opsystem == "Android" ]]; then
		pkg update -y;
		pkg install -y clang make ncurses-dev libevent-dev readline-dev;
	fi
elif [[ $machine == "Mac" ]]; then
	mkdir -p ./logs;
	brew update;
	if [[ ! -f $HOME/.brewconfig.zsh ]] && [[ $(uname -n) =~ 'unit.ua' ]]; then
		curl -fsSL https://rawgit.com/kube/42homebrew/master/install.sh | zsh;
	fi
	source ~/.zshrc;
	brew install libevent readline ncurses;
	brew link readline libevent ncurses;
	echo "export CPATH=\"$CPATH:$HOME/.brew/opt/readline/include/:$HOME/.brew/opt/ncurses/include/:$HOME/.brew/opt/libevent/include\"" >> ~/.zshrc;
	echo "export LIBRARY_PATH=\"$HOME/.brew/opt/readline/lib:$HOME/.brew/opt/ncurses/lib/:$HOME/.brew/opt/libevent/lib\"" >> ~/.zshrc;
else
	echo "Your system doesn't supported!";
fi
