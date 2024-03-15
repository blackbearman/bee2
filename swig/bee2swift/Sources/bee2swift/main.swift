import bee2

/// Test call function:  
//$ 200
print(beltHash_keep())

/// Test standard parameters
let buf = UnsafeMutableRawPointer.allocate(byteCount: 128, alignment: 4)
let sbuf = buf.bindMemory(to:CChar.self, capacity:128)

var params = bign_params()
var err = bignParamsStd(&params, "1.2.112.0.2.0.34.101.45.3.1")
// Print int:      
//$  128
print(params.l)

hexFrom(buf, &params.q, 32)
// Print octets:      
//$  07663D2699BF5A7EFC4DFB0DD68E5CD9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
print(String(cString:sbuf))

/// Test pubkey 
let privkey = UnsafeMutableRawPointer.allocate(byteCount: 64, alignment: 4)
let pubkey = UnsafeMutableRawPointer.allocate(byteCount: 128, alignment: 4)

hexTo(privkey, "1F66B5B84B7339674533F0329C74F21834281FED0732429E0C79235FC273E269")
err = bignPubkeyCalc(pubkey, &params, privkey)
print(err)
hexFrom(buf, pubkey, 64)
//$ BD1A5650179D79E03FCEE49D4C2BD5DDF54CE46D0CF11E4FF87BF7A890857FD0
//  7AC6A60361E8C8173491686D461B2826190C2EDA5909054A9AB84D2AB9D99A90
print(String(cString:sbuf))

/// Test hash
let hash = UnsafeMutableRawPointer.allocate(byteCount: 64, alignment: 4)
let src = beltH()
err = beltHash(hash, src, 13)
print(err)
hexFrom(buf, hash, 64)
//$ ABEF9725D4C5A83597A367D14494CC2542F20F659DDFECC961A3EC550CBA8C75
//  80000000000000F080720A2D81550000A89F9E04D27F0000C0908D04D27F0000
print(String(cString:sbuf))

/// Test DER
let der = UnsafeMutableRawPointer.allocate(byteCount: 512, alignment: 4)
var count: Int = 512
bignOidToDER(der, &count, "1.2.112.0.2.0.34.101.31.81")
//$ 11
print(count)

/// Test signature
let sig = UnsafeMutableRawPointer.allocate(byteCount: 64+32, alignment: 4)
err = bignSign2(sig, &params, der, count, hash, privkey, nil, 0)
print(err)
hexFrom(buf, sig, 16*3)
//$ 19D32B7E01E25BAE4A70EB6BCA42602CCA6A13944451BCC5D4C54CFD8737619C
//  328B8A58FB9C68FD17D569F7D06495FB
print(String(cString:sbuf))

// end
buf.deallocate()
privkey.deallocate()
pubkey.deallocate()
hash.deallocate()
der.deallocate()
sig.deallocate()