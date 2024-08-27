#pragma once


namespace core {

	template <class... Fs> struct Overload : Fs... { using Fs::operator()...; };
	template <class... Fs> Overload(Fs...) -> Overload<Fs...>;

};