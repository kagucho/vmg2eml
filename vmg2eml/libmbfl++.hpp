#include "mbfl/mbfilter.h"

#include <memory>
#include <functional>
#include <limits>
#include <new>
#include <cstring>
//mbfl_buffer_converter
namespace mbfl{
	class memory_device;

	class string {
	private:
		mbfl_string s_;
	public:
		string(mbfl_language_id no_language = mbfl_no_language_uni, mbfl_encoding_id no_encoding = no_encoding) noexcept
		{
			mbfl_string_init_set(&this->s_, no_language, no_encoding);
		}
		string(const string&) = delete;
		string(string&&) = default;
		string(memory_device&& d, mbfl_language_id no_language = mbfl_no_language_uni, mbfl_encoding_id no_encoding = no_encoding) noexcept;
		string& operator=(const string&) = delete;
		string& operator=(string&& o) noexcept {
			this->clear();
			this->s_ = o.s_;
		}
		unsigned char * data() noexcept { return this->s_.val; }
		const unsigned char * data() const noexcept { return this->s_.val; }
		unsigned int length() const noexcept { return this->s_.len; }
		mbfl_language_id language() const noexcept { return this->s_.no_language; }
		mbfl_no_encoding encoding() const noexcept { return this->s_.no_encoding; }
		mbfl_string& get() noexcept { return this->s_; }
		~string() noexcept {
			this->clear();
		}
		void clear() noexcept {
			mbfl_string_clear(&this->s_);
		}
	};
	class buffer_converter {
	private:
		using converter_t = std::unique_ptr<
			mbfl_buffer_converter,
			decltype(&mbfl_buffer_converter_delete)
		>;
		converter_t conv_;
	public:
		buffer_converter() = delete;
		buffer_converter(mbfl_no_encoding from, mbfl_no_encoding to, std::nothrow_t)
		: conv_(mbfl_buffer_converter_new(from, to, 0), mbfl_buffer_converter_delete)
		{}
		buffer_converter(mbfl_no_encoding from, mbfl_no_encoding to) : buffer_converter(from, to, std::nothrow)
		{
			if(!conv_) throw std::bad_alloc();
		}
		buffer_converter(const buffer_converter&) = delete;
		buffer_converter(buffer_converter&&) = default;
		buffer_converter& operator=(const buffer_converter&) = delete;
		buffer_converter& operator=(buffer_converter&&) = default;
		void reset() noexcept {
			mbfl_buffer_converter_reset(this->conv_.get());
		}
		bool illegal_mode(int mode) noexcept {
			return 0 != mbfl_buffer_converter_illegal_mode(
				this->conv_.get(), mode
			);
		}
		bool illegal_substchar(int substchar) noexcept {
			return 0 != mbfl_buffer_converter_illegal_substchar(
				this->conv_.get(), substchar
			);
		}
		int strncat(const unsigned char *p, int n) noexcept {
			return mbfl_buffer_converter_strncat(
				this->conv_.get(), p, n
			);
		}
		mbfl::string feed(mbfl_language_id no_language = mbfl_no_language_uni) {
			mbfl::string s(no_language);
			if(0 != mbfl_buffer_converter_feed(
				this->conv_.get(),
				&s.get()
			)) {
				throw std::_runtime_error("fail to convert");
			}
			return s;
		}
		mbfl::string feed(int& loc, mbfl_language_id no_language = mbfl_no_language_uni) noexcept {
			mbfl::string s(no_language);
			if(0 != mbfl_buffer_converter_feed2(
				this->conv_.get(),
				&s.get(),
				&loc
			)) {
				throw std::_runtime_error("fail to convert");
			}
			return s;
		}
		void flush() noexcept {
			mbfl_buffer_converter_flush(this->conv_.get());
		}
		mbfl::string getbuffer(mbfl_language_id no_language = mbfl_no_language_uni) {
			mbfl::string re(no_language);
			if(nullptr == mbfl_buffer_converter_getbuffer(
				this->conv_.get(),
				&re,
			)) {
				throw std::runtime_error("no buffer found");
			}
			return re;
		}
		mbfl::string result(mbfl_language_id no_language = mbfl_no_language_uni) {
			mbfl::string re(no_language);
			if(nullptr == mbfl_buffer_converter_result(
				this->conv_.get(),
				&re.get(),
			)) {
				throw std::runtime_error("fail to convert");
			}
			return re;
		}
		mbfl::string feed_result(mbfl::string& string, mbfl_language_id no_language = mbfl_no_language_uni) {
			mbfl::string re(no_language);
			if(nullptr == mbfl_buffer_converter_feed_result(
				this->conv_.get(),
				&string.get(),
				&result
			)) {
				throw std::runtime_error("fail to convert");
			}
			return re;
		}
	};
	class memory_device {
	private:
		mbfl_memory_device dev_;
	public:
		memory_device() = default;
		memory_device(int capacity, int allocate_size_min)
		{
			mbfl_memory_device_init(
				&this->dev_,
				std::max(0, capacity),
				std::max(allocate_size_min, MBFL_MEMORY_DEVICE_ALLOC_SIZE)
			);
			if(!this->dev_.buffer) throw std::bad_alloc();
		}
		memory_device(const memory_device&) = default;
		memory_device(memory_device&&) = default;
		~memory_device() noexcept {
			this->clear();
		}
		memory_device& operator=(const memory_device&) = default;
		memory_device& operator=(memory_device&& o) noexcept {
			this->clear();
			this->dev_ = o.dev_;
		}
		bool realloc(int capacity, int allocate_size_min = 0, std::nothrow_t) noexcept{
			mbfl_memory_device_realloc(
				&this->dev_,
				std::max(0, capacity),
				(allocate_size_min) ? std::max(allocate_size_min, MBFL_MEMORY_DEVICE_ALLOC_SIZE) : this->dev_.allocsz
			);
		}
		void realloc(int capacity, int allocate_size_min = 0) {
			this->realloc(capacity, allocate_size_min);
			if(!this->dev_.buffer) throw std::bad_alloc();
		}
		void clear() noexcept {
			mbfl_memory_device_clear(&this->dev_)
		}
		void reset() noexcept {
			this->dev_.pos = 0;
		}
		void unput() noexcept {
			if(0 < this->dev_.pos) --this->dev_.pos;
		}
		mbfl_string* result_move(mbfl_string& string) noexcept {
			//avoid string length overflow in mbfl_memory_device_result
			//https://bugs.php.net/bug.php?id=73505
			return ((std::numelic_limits<decltype(this->dev_->pos)>::max() - this->dev_->pos) < 4)
				? nullptr
				: mbfl_memory_device_result(&this->dev_, &string);
		}
		int strcat(const char *psrc, int len) noexcept {
			return ((std::numelic_limits<decltype(this->dev_->pos)>::max() - this->dev_->pos) < len)
				? -1
				: mbfl_memory_device_strncat(&this->dev_, psrc, len);
		}
		int strcat(const char *psrc) noexcept {
			return strcat(psrc, std::strlen(psrc))
		}
		int append(const memory_device& o) noexcept {
			mbfl_memory_device_devcat(&this->dev_, const_cast<mbfl_memory_device*>(&o));
		}
		unsigned char* data() noexcept { return this->dev_.buffer; }
		const unsigned char* data() const noexcept { return this->dev_.buffer; }
		int& size() noexcept { return this->dev_.pos; }
		int size() const noexcept { return this->dev_.pos; }
		int& capacity() noexcept { return this->dev_.length; }
		int capacity() const noexcept { return this->dev_.length; }
		int& allocate_size_min() noexcept { return this->dev_.allocsz; }
		int allocate_size_min() const noexcept { return this->dev_.allocsz; }
		void reserve(int s) {
			if(this->dev_.length < s) this->realloc(s);
		}
		void resize(int s) {
			this->reserve(s);
			this->dev_.pos = s;
		}
	};

	inline string::string(memory_device&& d, mbfl_language_id no_language, mbfl_encoding_id no_encoding) noexcept
	: string(no_language, no_encoding)
	{
		d.result_move(this->s_);
	}
}