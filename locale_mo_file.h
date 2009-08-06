#ifndef CPPCMS_LOCALE_MO_FILE_H
#define CPPCMS_LOCLAE_MO_FILE_H

#include <memory>

namespace cppcms {
	namespace locale {
		namespace impl {
			struct Localization_Text;

			class dictionary {
			public:
				~dictionary();
				char const *lookup(char const *s,int std_id) const;
				static dictionary *load(char const *file_name);
			private:
				dictionary(std::auto_ptr<Localization_Text>);
				dictionary(dictionary const &);
				dictionary const &operator=(dictionary const &);

				std::auto_ptr<Localization_Text> loc_;
			};
		}
	}

} 



#endif
