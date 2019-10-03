#pragma once

#include <wink/signal.hpp>
#include <wink/event_queue.hpp>

namespace BitEngine {

	template<class Format>
	using SignalHandler = wink::slot<Format>;

	template<class Format> 
	using Signal = wink::signal<SignalHandler<Format>>;

	/// Used to create scoped connectors. When the scope finishes, the connection is automagically removed.
	/// Usage as: ScopedConnector<Format>  myscope(signal, [func, lambda, method]);
	template<class Format>
	class ScopedConnector {
		Signal<Format>& sig;
		SignalHandler<Format> slot;

		// No copy
		ScopedConnector(const ScopedConnector& a) = delete;
		ScopedConnector& operator=(const ScopedConnector& a) = delete;

		public:
			ScopedConnector(ScopedConnector&& a) = default;
			ScopedConnector& operator=(ScopedConnector&& a) = default;

			template<typename... Args>
			ScopedConnector(Signal<Format> &signal, Args&&... args)
				: sig(signal), slot(std::forward<Args>(args)...) {
				sig.connect(slot);
			}

			~ScopedConnector() {
				sig.disconnect(slot);
			}
	};
}