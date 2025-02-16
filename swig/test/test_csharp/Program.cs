using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Reflection;
using bcrypto;

namespace MyApp
{
    internal class Program
    {
        static void Main(string[] args)
        {
            Bee2Loader.Initialize();
            // begin
            SWIGTYPE_p_void buf = bee2.memAlloc(128);

            // test keep functions
            Console.WriteLine(bee2.beltHash_keep());

            // hexFrom test (B194BAC80A08F53B366D008E584A5DE4)
            SWIGTYPE_p_void sbuf = bee2.memAlloc(100);
            bee2.hexFrom(bee2.getStr(sbuf), bee2.op2vp(bee2.beltH()), 16);
            Console.WriteLine(bee2.vp2cp(sbuf));

            // hexTo test
            bee2.hexTo(buf, "69CCA1C93557C9E3D66BC3E0FA88FA6E");
            bee2.hexFrom(bee2.getStr(sbuf), buf, 16);
            Console.WriteLine(bee2.vp2cp(sbuf));

            bign_params b = new bign_params();
            uint err = bee2.bignParamsStd(b, "1.2.112.0.2.0.34.101.45.3.1");
            Console.WriteLine("Std " + err);
            Console.WriteLine("Bign l " + b.l);
     
            // "07663D2699BF5A7EFC4DFB0DD68E5CD9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            bee2.hexFrom(bee2.getStr(sbuf), bee2.op2vp(b.q), 32);
            Console.WriteLine("Bign q " + bee2.vp2cp(sbuf));

            // Test pubkey 
            SWIGTYPE_p_void privkey = bee2.memAlloc(64);
            SWIGTYPE_p_void pubkey = bee2.memAlloc(128);

            bee2.hexTo(privkey, "1F66B5B84B7339674533F0329C74F21834281FED0732429E0C79235FC273E269");
            err = bee2.bignPubkeyCalc(bee2.vp2op(pubkey), b, bee2.vp2op(privkey));
            Console.WriteLine("Pubkey " + err);
            // "BD1A5650179D79E03FCEE49D4C2BD5DDF54CE46D0CF11E4FF87BF7A890857FD0
            //  7AC6A60361E8C8173491686D461B2826190C2EDA5909054A9AB84D2AB9D99A90"
            bee2.hexFrom(bee2.getStr(sbuf), pubkey, 64);
            Console.WriteLine(bee2.vp2cp(sbuf));

            // Test hash 
            SWIGTYPE_p_void hash = bee2.memAlloc(64);
            err = bee2.beltHash(bee2.vp2op(hash), bee2.op2vp(bee2.beltH()), 13);
            Console.WriteLine("Hash " + err);
            // "ABEF9725D4C5A83597A367D14494CC2542F20F659DDFECC961A3EC550CBA8C75"
            bee2.hexFrom(bee2.getStr(sbuf), hash, 32);
            Console.WriteLine(bee2.vp2cp(sbuf));

            // Test DER
            SWIGTYPE_p_void sig = bee2.memAlloc(64+32);
            SWIGTYPE_p_void der = bee2.memAlloc(512);
            SWIGTYPE_p_size_t count = bee2.new_sizeTarr(1);

            bee2.sizeTarr_setitem(count, 0, 512);
            bee2.bignOidToDER(bee2.vp2op(der), count, "1.2.112.0.2.0.34.101.31.81");
            uint c1 = bee2.sizeTarr_getitem(count, 0);
            Console.WriteLine("DER " + c1);

            // Test signature
            err = bee2.bignSign2(bee2.vp2op(sig), b, bee2.vp2op(der), c1, bee2.vp2op(hash), bee2.vp2op(privkey), null, 0);
            Console.WriteLine("Sign " + err);

            // "19D32B7E01E25BAE4A70EB6BCA42602CCA6A13944451BCC5D4C54CFD8737619C
            //  328B8A58FB9C68FD17D569F7D06495FB"
            bee2.hexFrom(bee2.getStr(sbuf), sig, 16*3);
            Console.WriteLine(bee2.vp2cp(sbuf));
            err = bee2.bignVerify(b, bee2.vp2op(der), c1, bee2.vp2op(hash), bee2.vp2op(sig), bee2.vp2op(pubkey));
            Console.WriteLine("Verify " + err);

            // End
            bee2.memFree(buf);
            bee2.memFree(sbuf);
            bee2.memFree(privkey);
            bee2.memFree(pubkey);
            bee2.memFree(hash);
            bee2.memFree(sig);
            bee2.memFree(der);
            bee2.delete_sizeTarr(count);
        }
    }
}
