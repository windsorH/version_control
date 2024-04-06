
#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

namespace openssl_rsa_3_x
{
    // 官方文档 https://www.openssl.org/docs/man3.1/man7/EVP_PKEY-RSA.html
    // 下面的 GenerateKeys1/2/3, 都能用, 1和2效果是一样的, 3可以设置参数生成密钥

    // 从EVP_PKEY对象中提取公钥和私钥
    inline void GenerateKeysForEvp(EVP_PKEY* pkey, std::string& pri_key, std::string& pub_key)
    {
        BIO* pri = BIO_new(BIO_s_mem());
        BIO* pub = BIO_new(BIO_s_mem());

        PEM_write_bio_PrivateKey(pri, pkey, NULL, NULL, 0, NULL, NULL);
        PEM_write_bio_PUBKEY(pub, pkey);

        size_t pri_len = BIO_pending(pri);
        size_t pub_len = BIO_pending(pub);

        pri_key.resize(pri_len);
        pub_key.resize(pub_len);
        BIO_read(pri, &pri_key[0], pri_len);
        BIO_read(pub, &pub_key[0], pub_len);

        // 内存释放  
        BIO_free_all(pub);
        BIO_free_all(pri);
    }

    // 生成密钥对, 返回是否成功
    // pri_key = 接收私钥结果
    // pub_key = 接收公钥结果
    // bits = 密钥长度
    inline bool GenerateKeys1(std::string& pri_key, std::string& pub_key, UINT bits = 2048)
    {
        EVP_PKEY* pkey = EVP_RSA_gen(bits);
        if (!pkey)
            return false;

        GenerateKeysForEvp(pkey, pri_key, pub_key);

        EVP_PKEY_free(pkey);
        return true;
    }

    // 生成密钥对, 返回是否成功
    // pri_key = 接收私钥结果
    // pub_key = 接收公钥结果
    // bits = 密钥长度
    inline bool GenerateKeys2(std::string& pri_key, std::string& pub_key, UINT bits = 2048)
    {
        EVP_PKEY* pkey = NULL;
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
        EVP_PKEY_keygen_init(pctx);
        EVP_PKEY_generate(pctx, &pkey);
        if (!pkey)
            return false;

        GenerateKeysForEvp(pkey, pri_key, pub_key);

        EVP_PKEY_CTX_free(pctx);
        return true;
    }

    // 生成密钥对, 返回是否成功
    // pri_key = 接收私钥结果
    // pub_key = 接收公钥结果
    // bits = 密钥长度
    inline bool GenerateKeys3(std::string& pri_key, std::string& pub_key, UINT bits = 2048)
    {
        unsigned int primes = 3;
        OSSL_PARAM params[3];
        EVP_PKEY* pkey = NULL;
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);

        EVP_PKEY_keygen_init(pctx);

        // 设置参数生成密钥
        // 官方文档 https://www.openssl.org/docs/man3.0/man3/OSSL_PARAM_construct_uint.html
        params[0] = OSSL_PARAM_construct_uint("bits", &bits);
        params[1] = OSSL_PARAM_construct_uint("primes", &primes);
        params[2] = OSSL_PARAM_construct_end();
        EVP_PKEY_CTX_set_params(pctx, params);

        EVP_PKEY_generate(pctx, &pkey);

        if (!pkey)
        {
            EVP_PKEY_CTX_free(pctx);
            return false;
        }

        GenerateKeysForEvp(pkey, pri_key, pub_key);

        EVP_PKEY_CTX_free(pctx);
        return true;
    }

    // 一般情况下都是公钥加密, 私钥解密, 如果是私钥加密的话, 私钥也可以解密
    // 所以只写公钥加密和私钥解密的函数
    // 需要私钥解密公钥解密的话, 可以改 PEM_read_bio_PUBKEY/PEM_read_bio_PrivateKey
    // 看名字就能看出来, 一个是读取公钥, 一个是读取私钥
    // 后续的加解密操作都是根据 PEM_read_bio_PUBKEY/PEM_read_bio_PrivateKey 返回的对象操作的

    // 公钥加密
    // pData = 要加密的数据
    // nSize = 要加密的数据长度
    // pub_key = 公钥
    inline std::string rsa_encrypt(LPCVOID pData, size_t nSize, LPCSTR pub_key)
    {
        std::string en_data;
        if (!pData || nSize == 0)
            return en_data;   // 没有数据, 直接返回空字符串

        EVP_PKEY_CTX* ctx = 0;
        BIO* keybio = BIO_new_mem_buf(pub_key, -1);
        EVP_PKEY* pKey = PEM_read_bio_PUBKEY(keybio, 0, NULL, NULL);
        while (pKey)
        {
            ctx = EVP_PKEY_CTX_new(pKey, NULL);
            if (!ctx)
                break;

            if (EVP_PKEY_encrypt_init(ctx) <= 0)  // 初始化加密
                break;

            const int key_len = EVP_PKEY_size(pKey);
            char* buf = new char[key_len];

            LPCBYTE ptr = reinterpret_cast<LPCBYTE>(pData);
            LPCBYTE pEnd = ptr + nSize;            const int block_len = key_len - 11;
            int len = block_len;

            // 计算结果长度, 预先分配这么大的内存, 计算方式为:
            // 每次加密最大尺寸 = block_len
            // 加密次数 = 总尺寸 / block_len
            // 每次加密得到的长度 = key_len
            // 总尺寸 = 每次加密得到的长度 * 总次数
            const size_t reserve_size = ((nSize / block_len + ((nSize % block_len == 0) ? 0 : 1)) * key_len);
            en_data.reserve(reserve_size);

            while (ptr < pEnd)
            {
                if ((ptr + len) > pEnd)
                    len = pEnd - ptr;

                size_t outl = block_len;
                int ret = EVP_PKEY_encrypt(ctx, reinterpret_cast<LPBYTE>(buf), &outl, ptr, len);
                if (ret > 0)
                {
                    en_data.append(buf, outl);
                }
                ptr += len;
            }
            delete[] buf;

            break;
        }

        if (pKey)
            EVP_PKEY_free(pKey);
        if (ctx)
            EVP_PKEY_CTX_free(ctx);

        return en_data;

    }

    // 私钥解密
    // pData = 要加密的数据
    // nSize = 要加密的数据长度
    // pub_key = 私钥
    inline std::string rsa_decrypt(LPCVOID pData, size_t nSize, LPCSTR pri_key)
    {
        std::string de_data;
        if (!pData || nSize == 0)
            return de_data;   // 没有数据, 直接返回空字符串

        EVP_PKEY_CTX* ctx = 0;
        BIO* keybio = BIO_new_mem_buf(pri_key, -1);
        EVP_PKEY* pKey = PEM_read_bio_PrivateKey(keybio, 0, NULL, NULL);

        while (pKey)
        {
            ctx = EVP_PKEY_CTX_new(pKey, NULL);
            if (!ctx)
                break;

            if (EVP_PKEY_decrypt_init(ctx) <= 0)  // 初始化解密
                break;

            // 解密的长度就不计算了, 直接使用数据长度, 解密的尺寸肯定小于数据长度
            de_data.reserve(nSize);
            LPCBYTE ptr = reinterpret_cast<LPCBYTE>(pData);
            LPCBYTE pEnd = ptr + nSize;            const int key_len = EVP_PKEY_size(pKey);
            int len = key_len;
            char* buf = new char[key_len + 1];

            while (ptr < pEnd)
            {
                if ((ptr + len) > pEnd)
                    len = pEnd - ptr;

                size_t outl = key_len;
                int ret = EVP_PKEY_decrypt(ctx, reinterpret_cast<LPBYTE>(buf), &outl, ptr, len);
#if defined(_DEBUG) || defined(DEBUG)
                if (ret <= 0)
                {
                    unsigned long err = ERR_get_error(); //获取错误号
                    char err_msg[1024] = { 0 };
                    ERR_error_string(err, err_msg); // 格式：error:errId:库:函数:原因
                    printf("err msg: err:%ld, msg:%s\n", err, err_msg);
                }
#endif
                if (ret > 0)
                {
                    de_data.append(buf, outl);
                }
                ptr += len;
            }
            delete[] buf;

            break;
        }

        if (pKey)
            EVP_PKEY_free(pKey);
        if (ctx)
            EVP_PKEY_CTX_free(ctx);

        return de_data;
    }

}


int main()
{
    std::string pri_key, pub_key;

    // 生成密钥对
    openssl_rsa_3_x::GenerateKeys1(pri_key, pub_key);

    // 公钥加密, 加密私钥这段数据
    std::string en_data = openssl_rsa_3_x::rsa_encrypt(pri_key.c_str(), pri_key.size(), pub_key.c_str());

    // 私钥解密
    std::string de_data = openssl_rsa_3_x::rsa_decrypt(en_data.c_str(), en_data.size(), pri_key.c_str());

    bool b = de_data == pri_key;

    std::cout << "解密后的值和加密前的值对比结果: " << b << std::endl;

    return 0;

}