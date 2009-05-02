#ifndef CPPCMS_CONTEXT_H
#define CPPCMS_CONTEXT_H

namespace cppcms {

	class context {
	public:
		http::request &request();
		http::respnse &response();
		cppcms::session &session();
	};


} // namespace cppcms


#endif
