#include "transtext.h"

using namespace std;

namespace cppcms {

namespace transtext {

static const trans default_trans;

trans const &trans_factory::get(string lang,string domain) const
{
	map<string,map<string,boost::shared_ptr<trans> > >::const_iterator p;

	if(lang.empty()) lang=default_lang;
	if(domain.empty()) domain=default_domain;

	if((p=langs.find(lang))==langs.end()) {
		return default_trans;
	}
	else {
		map<string,boost::shared_ptr<trans> >::const_iterator p2;
		if((p2=p->second.find(domain))==p->second.end()) {
			return default_trans;
		}
		return *(p2->second);
	}
}

void trans_factory::load(
			string const &locale_dir,
			vector<string> const &lang_list,
			string const &lang_def,
			vector<string> const &domain_list,
			string const &domain_def)
{
	if(lang_list.empty() || domain_list.empty()) {
		return;
	}

	default_lang=lang_def;
	default_domain=domain_def;

	if(default_lang.empty()) default_lang=lang_list[0];
	if(default_domain.empty()) default_domain=domain_list[0];

	typedef vector<string>::const_iterator it_t;

	for(it_t lang=lang_list.begin(),le=lang_list.end();lang!=le;++lang)
	{
		for(it_t domain=domain_list.begin(),de=domain_list.end();de!=domain;++domain) {
			boost::shared_ptr<trans_thread_safe> tr(new trans_thread_safe());
			tr->load(lang->c_str(),domain->c_str(),locale_dir.c_str());
			langs[*lang][*domain]=tr;		
		}
	}
}


trans_factory::~trans_factory()
{
}


} // namespace transtext

} // namespace cppcms
